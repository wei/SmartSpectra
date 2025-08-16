export class NetworkClient {
  private apiKey: string = '';
  private serverUrl: string = '';
  private sessionId: string | null = null;
  private webSocket: WebSocket | null = null;

  configure(serverUrl: string, apiKey: string): void {
    this.serverUrl = serverUrl;
    this.apiKey = apiKey;
  }

  async initializeSession(config: any): Promise<string> {
    const res = await fetch(`${this.serverUrl}/initialize`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ apiKey: this.apiKey, ...config })
    });
    const json = await res.json();
    this.sessionId = json.sessionId;
    return this.sessionId!;
  }

  async startSession(): Promise<void> {
    if (!this.sessionId) throw new Error('No session');
    await fetch(`${this.serverUrl}/start`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sessionId: this.sessionId })
    });
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

  connectWebSocket(onMessage: (data: any) => void): void {
    if (!this.sessionId) throw new Error('No session');
    this.webSocket = new WebSocket(`${this.serverUrl.replace('http', 'ws')}/process?sessionId=${this.sessionId}`);
    this.webSocket.onmessage = (event) => {
      const data = JSON.parse(event.data.toString());
      onMessage(data);
    };
  }

  sendFrame(frame: Blob): void {
    if (this.webSocket && this.webSocket.readyState === WebSocket.OPEN) {
      this.webSocket.send(frame);
    }
  }

  disconnect(): void {
    this.webSocket?.close();
    this.webSocket = null;
  }
}
