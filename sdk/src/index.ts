import { NetworkClient } from './networkClient';
import { VideoCaptureManager } from './videoCapture';
import { SmartSpectraError, ErrorCode } from './errors';

export enum SmartSpectraMode {
  SPOT = 'spot',
  CONTINUOUS = 'continuous'
}

export interface SmartSpectraConfig {
  apiKey: string;
  serverUrl: string;
  mode: SmartSpectraMode;
  spotDuration?: number;
  bufferDuration?: number;
}

export interface MetricsBuffer {
  pulse?: any;
  breathing?: any;
  face?: any;
}

export interface SmartSpectraCallbacks {
  onResult?: (result: MetricsBuffer) => void;
  onStatus?: (status: string) => void;
  onError?: (error: SmartSpectraError) => void;
}

export interface StartOptions extends SmartSpectraCallbacks {
  mode: SmartSpectraMode;
  duration?: number;
}

export class SmartSpectraWebSDK {
  private static instance: SmartSpectraWebSDK;
  static getInstance() {
    if (!this.instance) this.instance = new SmartSpectraWebSDK();
    return this.instance;
  }

  private network = new NetworkClient();
  private video = new VideoCaptureManager();
  private config?: SmartSpectraConfig;
  private callbacks: SmartSpectraCallbacks = {};
  private running = false;
  private frameInterval: any;
  private metrics: MetricsBuffer | null = null;

  async configure(config: SmartSpectraConfig): Promise<void> {
    this.config = config;
    this.network.configure(config.serverUrl, config.apiKey);
    await this.network.initializeSession(config);
  }

  setApiKey(apiKey: string) {
    if (!this.config) this.config = { apiKey, serverUrl: '', mode: SmartSpectraMode.SPOT };
    this.config.apiKey = apiKey;
  }

  setServerUrl(url: string) {
    if (!this.config) this.config = { apiKey: '', serverUrl: url, mode: SmartSpectraMode.SPOT };
    this.config.serverUrl = url;
  }

  async setVideoElement(videoElement: HTMLVideoElement): Promise<void> {
    await this.video.initialize(videoElement);
  }

  async requestCameraPermission(): Promise<MediaStream> {
    const stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false });
    return stream;
  }

  async start(options: StartOptions): Promise<void> {
    if (this.running) throw new SmartSpectraError('Already running', ErrorCode.ALREADY_RUNNING, 'runtime');
    if (!this.config) throw new SmartSpectraError('Not configured', ErrorCode.NOT_INITIALIZED, 'initialization');
    this.callbacks = options;
    await this.network.startSession();
    this.network.connectWebSocket((msg) => this.handleMessage(msg));
    this.running = true;
    this.frameInterval = setInterval(async () => {
      const frame = await this.video.encodeFrame('image/jpeg', 0.8);
      this.network.sendFrame(frame);
    }, 1000);
  }

  private handleMessage(msg: any) {
    if (msg.type === 'metrics') {
      this.metrics = msg.data;
      this.callbacks.onResult?.(msg.data);
    } else if (msg.type === 'status') {
      this.callbacks.onStatus?.(msg.data);
    } else if (msg.type === 'error') {
      const err = new SmartSpectraError(msg.data.message, ErrorCode.PROCESSING_FAILED, 'runtime');
      this.callbacks.onError?.(err);
    }
  }

  async stop(): Promise<void> {
    if (!this.running) throw new SmartSpectraError('Not running', ErrorCode.NOT_RUNNING, 'runtime');
    clearInterval(this.frameInterval);
    this.network.disconnect();
    await this.network.stopSession();
    this.running = false;
  }

  get metricsBuffer() {
    return this.metrics;
  }
}
