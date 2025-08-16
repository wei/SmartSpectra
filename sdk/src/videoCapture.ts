export class VideoCaptureManager {
  private canvas: HTMLCanvasElement;
  private context: CanvasRenderingContext2D;
  private videoElement: HTMLVideoElement;

  constructor() {
    this.canvas = document.createElement('canvas');
    this.context = this.canvas.getContext('2d') as CanvasRenderingContext2D;
    this.videoElement = document.createElement('video');
    this.videoElement.autoplay = true;
  }

  async initialize(videoElement: HTMLVideoElement): Promise<void> {
    this.videoElement = videoElement;
  }

  async encodeFrame(format: 'image/jpeg' = 'image/jpeg', quality: number = 0.8): Promise<Blob> {
    const { videoWidth, videoHeight } = this.videoElement;
    this.canvas.width = videoWidth;
    this.canvas.height = videoHeight;
    this.context.drawImage(this.videoElement, 0, 0, videoWidth, videoHeight);
    return new Promise((resolve) => this.canvas.toBlob((b) => resolve(b as Blob), format, quality));
  }

  getFrameDimensions() {
    return { width: this.videoElement.videoWidth, height: this.videoElement.videoHeight };
  }

  getCurrentTimestamp(): number {
    return Date.now();
  }
}
