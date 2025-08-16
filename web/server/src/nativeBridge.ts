export interface NativeContainer {
  initialize(apiKey: string, config: any, onMetrics: (json: string, timestamp: number) => void, onStatus: (status: string) => void): void;
  start(): void;
  stop(): void;
  addFrame(frame: Buffer, timestamp: number): void;
}

// eslint-disable-next-line @typescript-eslint/no-var-requires
const addon: { Container: { new(): NativeContainer } } = require('../build/Release/smartspectra.node');

export function createContainer(): NativeContainer {
  return new addon.Container();
}
