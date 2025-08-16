export enum SmartSpectraMode {
  SPOT = 'spot',
  CONTINUOUS = 'continuous'
}

export interface SmartSpectraConfig {
  apiKey: string;
  serverUrl: string;
  mode: SmartSpectraMode;
}

export interface Measurement {
  time: number;
  value: number;
  stable: boolean;
}

export interface MeasurementWithConfidence extends Measurement {
  confidence: number;
}

export interface MetricsBuffer {
  pulse?: {
    rate: MeasurementWithConfidence[];
  };
  breathing?: {
    rate: MeasurementWithConfidence[];
  };
  face?: {
    blinkCount: number;
  };
}

export interface StartOptions {
  mode: SmartSpectraMode;
  onResult?: (result: MetricsBuffer) => void;
}
