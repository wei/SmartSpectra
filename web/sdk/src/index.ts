import { SmartSpectraConfig, SmartSpectraMode, MetricsBuffer, StartOptions } from './types';
import { VideoCaptureManager } from './VideoCaptureManager';
import { NetworkClient } from './NetworkClient';

export class SmartSpectraWebSDK {
  private static instance: SmartSpectraWebSDK;
  private config: SmartSpectraConfig | null = null;
  private videoCapture = new VideoCaptureManager();
  private networkClient = new NetworkClient();
  private frameInterval: any = null;
  public onResult?: (result: MetricsBuffer) => void;

  static getInstance(): SmartSpectraWebSDK {
    if (!SmartSpectraWebSDK.instance) {
      SmartSpectraWebSDK.instance = new SmartSpectraWebSDK();
    }
    return SmartSpectraWebSDK.instance;
  }

  async configure(config: SmartSpectraConfig): Promise<void> {
    this.config = config;
    this.networkClient.configure(config.serverUrl, config.apiKey);
    await this.networkClient.initializeSession({ mode: config.mode });
  }

  async setVideoElement(videoElement: HTMLVideoElement): Promise<void> {
    await this.videoCapture.initialize(videoElement);
  }

  async start(options: StartOptions): Promise<void> {
    if (!this.config) throw new Error('Not configured');
    await this.networkClient.startSession();
    this.networkClient.connectWebSocket((data) => {
      if (data.type === 'metrics' && this.onResult) {
        this.onResult(data.data as MetricsBuffer);
      }
    });
    this.frameInterval = setInterval(async () => {
      const blob = await this.videoCapture.encodeFrame('image/jpeg', 0.8);
      this.networkClient.sendFrame(blob);
    }, 500);
  }

  async stop(): Promise<void> {
    clearInterval(this.frameInterval);
    this.networkClient.disconnect();
    await this.networkClient.stopSession();
  }
}

export * from './types';
