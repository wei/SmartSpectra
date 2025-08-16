import express from 'express';
import cors from 'cors';
import bodyParser from 'body-parser';
import { WebSocketServer } from 'ws';
import { v4 as uuidv4 } from 'uuid';
import axios from 'axios';

interface Session {
  id: string;
  apiKey: string;
  config: any;
  started: boolean;
  blinkCount: number;
}

const sessions = new Map<string, Session>();

const app = express();
app.use(cors());
app.use(bodyParser.json());

app.post('/initialize', (req, res) => {
  const { apiKey, ...config } = req.body;
  if (!apiKey) {
    return res.status(400).json({ error: 'apiKey required' });
  }
  const sessionId = uuidv4();
  sessions.set(sessionId, { id: sessionId, apiKey, config, started: false, blinkCount: 0 });
  res.json({ sessionId });
});

app.post('/start', (req, res) => {
  const { sessionId } = req.body;
  const session = sessions.get(sessionId);
  if (!session) {
    return res.status(404).json({ error: 'session not found' });
  }
  session.started = true;
  res.json({ ok: true });
});

app.post('/stop', (req, res) => {
  const { sessionId } = req.body;
  const session = sessions.get(sessionId);
  if (!session) {
    return res.status(404).json({ error: 'session not found' });
  }
  sessions.delete(sessionId);
  res.json({ ok: true });
});

app.get('/status', (req, res) => {
  const sessionId = req.query.sessionId as string;
  const session = sessions.get(sessionId);
  if (!session) {
    return res.status(404).json({ error: 'session not found' });
  }
  res.json({ started: session.started });
});

const server = app.listen(8080, () => {
  console.log('Server listening on port 8080');
});

const wss = new WebSocketServer({ server, path: '/process' });

wss.on('connection', (ws, req) => {
  const url = new URL(req.url || '', `http://${req.headers.host}`);
  const sessionId = url.searchParams.get('sessionId');
  if (!sessionId || !sessions.has(sessionId)) {
    ws.close();
    return;
  }
  const session = sessions.get(sessionId)!;

  ws.on('message', async (data: Buffer) => {
    // In a real implementation, frames would be decoded and processed by C++ SDK.
    // Here we simulate processing and call Presage API placeholder.
    try {
      await axios.post('https://example.com/presage', {}, {
        headers: { Authorization: `Bearer ${session.apiKey}` },
        timeout: 1000
      }).catch(() => {});
    } catch (err) {
      console.error('Presage API call failed', err);
    }
    session.blinkCount += 1;
    const metrics = {
      pulse: { rate: [{ time: Date.now(), value: 60 + Math.random() * 20, stable: true, confidence: 0.95 }] },
      breathing: { rate: [{ time: Date.now(), value: 12 + Math.random() * 4, stable: true, confidence: 0.9 }] },
      face: { blinkCount: session.blinkCount }
    };
    ws.send(JSON.stringify({ type: 'metrics', data: metrics }));
  });
});

