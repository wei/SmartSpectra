import express from 'express';
import { WebSocketServer } from 'ws';
import { v4 as uuidv4 } from 'uuid';
import { sessions, Session } from './session';
import http from 'http';
import { createContainer } from './nativeBridge';

const app = express();
app.use(express.json());

// Initialize a new session
app.post('/initialize', (req, res) => {
  const { apiKey, ...config } = req.body;
  if (!apiKey) {
    return res.status(400).json({ error: 'apiKey required' });
  }
  const id = uuidv4();
  const container = createContainer();
  const session: Session = {
    id,
    apiKey,
    config,
    container,
    running: false,
    lastStatus: 'PROCESSING_NOT_STARTED'
  };
  sessions.set(id, session);

  container.initialize(apiKey, config,
    (metricsJson: string) => {
      const s = sessions.get(id);
      if (s?.websocket) {
        const metrics = JSON.parse(metricsJson);
        s.websocket.send(JSON.stringify({ type: 'metrics', data: metrics }));
      }
    },
    (status: string) => {
      const s = sessions.get(id);
      if (s) {
        s.lastStatus = status;
        s.websocket?.send(JSON.stringify({ type: 'status', data: status }));
      }
    }
  );

  res.json({ sessionId: id });
});

// Start session
app.post('/start', (req, res) => {
  const { sessionId } = req.body;
  const session = sessions.get(sessionId);
  if (!session) return res.status(404).json({ error: 'session not found' });
  session.running = true;
  session.container.start();
  res.json({ ok: true });
});

// Stop session
app.post('/stop', (req, res) => {
  const { sessionId } = req.body;
  const session = sessions.get(sessionId);
  if (!session) return res.status(404).json({ error: 'session not found' });
  session.running = false;
  session.container.stop();
  session.websocket?.close();
  res.json({ ok: true });
});

// Status
app.get('/status', (req, res) => {
  const { sessionId } = req.query as { sessionId?: string };
  const session = sessionId ? sessions.get(sessionId) : undefined;
  if (!session) return res.status(404).json({ error: 'session not found' });
  res.json({ running: session.running, status: session.lastStatus });
});

const server = http.createServer(app);
const wss = new WebSocketServer({ noServer: true });

wss.on('connection', (ws, request) => {
  const url = new URL(request.url ?? '', `http://${request.headers.host}`);
  const sessionId = url.searchParams.get('sessionId');
  if (!sessionId) {
    ws.close();
    return;
  }
  const session = sessions.get(sessionId);
  if (!session) {
    ws.close();
    return;
  }
  session.websocket = ws;

  ws.on('message', (data: Buffer) => {
    if (!session.running) return;
    const timestamp = Date.now();
    session.container.addFrame(Buffer.from(data), timestamp);
  });

  ws.on('close', () => {
    session.running = false;
  });
});

server.on('upgrade', (request, socket, head) => {
  const url = new URL(request.url ?? '', `http://${request.headers.host}`);
  if (url.pathname === '/process') {
    wss.handleUpgrade(request, socket, head, (ws) => {
      wss.emit('connection', ws, request);
    });
  } else {
    socket.destroy();
  }
});

const PORT = process.env.PORT || 8080;
server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
