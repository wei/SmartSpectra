import { SmartSpectraError, ErrorCode } from './errors';

export class NetworkClient {
  private apiKey = '';
  private serverUrl = '';
  private sessionId: string | null = null;
  private webSocket: WebSocket | null = null;

  configure(serverUrl: string, apiKey: string) {
    this.serverUrl = serverUrl;
    this.apiKey = apiKey;
  }

  async initializeSession(config: any): Promise<string> {
    const res = await fetch(`${this.serverUrl}/initialize`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ apiKey: this.apiKey, ...config })
    });
    if (!res.ok) throw new SmartSpectraError('Failed to initialize', ErrorCode.NETWORK_ERROR, 'network');
    const data = await res.json();
    this.sessionId = data.sessionId;
    return data.sessionId;
  }

  async startSession(): Promise<void> {
    if (!this.sessionId) throw new SmartSpectraError('No session', ErrorCode.INVALID_CONFIG, 'configuration');
    const res = await fetch(`${this.serverUrl}/start`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sessionId: this.sessionId })
    });
    if (!res.ok) throw new SmartSpectraError('Failed to start', ErrorCode.NETWORK_ERROR, 'network');
  }

  async stopSession(): Promise<void> {
    if (!this.sessionId) return;
    await fetch(`${this.serverUrl}/stop`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sessionId: this.sessionId })
    });
    this.sessionId = null;
  }

  connectWebSocket(onMessage: (data: any) => void) {
    if (!this.sessionId) throw new SmartSpectraError('No session', ErrorCode.INVALID_CONFIG, 'configuration');
    this.webSocket = new WebSocket(`${this.serverUrl.replace('http', 'ws')}/process?sessionId=${this.sessionId}`);
    this.webSocket.onmessage = (event) => {
      onMessage(JSON.parse(event.data.toString()));
    };
  }

  sendFrame(frame: Blob) {
    this.webSocket?.send(frame);
  }

  disconnect() {
    this.webSocket?.close();
    this.webSocket = null;
  }
}
