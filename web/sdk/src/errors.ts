export type ErrorCategory = 'initialization' | 'configuration' | 'runtime' | 'network';

export enum ErrorCode {
  SERVER_CONNECTION_FAILED = 'SERVER_CONNECTION_FAILED',
  WEBSOCKET_ERROR = 'WEBSOCKET_ERROR',
  CAMERA_ACCESS_DENIED = 'CAMERA_ACCESS_DENIED',
  INVALID_API_KEY = 'INVALID_API_KEY',
  NETWORK_ERROR = 'NETWORK_ERROR',
  PROCESSING_FAILED = 'PROCESSING_FAILED',
  INVALID_CONFIG = 'INVALID_CONFIG',
  NOT_INITIALIZED = 'NOT_INITIALIZED',
  ALREADY_RUNNING = 'ALREADY_RUNNING',
  NOT_RUNNING = 'NOT_RUNNING',
  UNSUPPORTED_BROWSER = 'UNSUPPORTED_BROWSER'
}

export class SmartSpectraError extends Error {
  constructor(
    message: string,
    public code: ErrorCode,
    public category: ErrorCategory,
    public recoverable: boolean = false,
    public cause?: Error
  ) {
    super(message);
    this.name = 'SmartSpectraError';
  }
}

export interface ErrorReport {
  error: SmartSpectraError;
  timestamp: number;
  context: Record<string, any>;
}
