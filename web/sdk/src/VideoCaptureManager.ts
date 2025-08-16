export class VideoCaptureManager {
  private canvas: HTMLCanvasElement;
  private context: CanvasRenderingContext2D;
  private videoElement!: HTMLVideoElement;

  constructor() {
    this.canvas = document.createElement('canvas');
    const ctx = this.canvas.getContext('2d');
    if (!ctx) {
      throw new Error('Canvas not supported');
    }
    this.context = ctx;
  }

  async initialize(videoElement: HTMLVideoElement): Promise<void> {
    this.videoElement = videoElement;
    this.canvas.width = videoElement.videoWidth;
    this.canvas.height = videoElement.videoHeight;
  }

  async encodeFrame(format: 'image/jpeg' = 'image/jpeg', quality: number = 0.8): Promise<Blob> {
    this.context.drawImage(this.videoElement, 0, 0, this.canvas.width, this.canvas.height);
    return await new Promise<Blob>((resolve) => this.canvas.toBlob(blob => resolve(blob!), format, quality));
  }

  getFrameDimensions(): { width: number; height: number } {
    return { width: this.canvas.width, height: this.canvas.height };
  }

  getCurrentTimestamp(): number {
    return Date.now();
  }
}
