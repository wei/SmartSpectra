import { WebSocket } from 'ws';
import { NativeContainer } from './nativeBridge';

export interface Session {
  id: string;
  apiKey: string;
  config: any;
  container: NativeContainer;
  websocket?: WebSocket;
  running: boolean;
  lastStatus: string;
}

export const sessions = new Map<string, Session>();
