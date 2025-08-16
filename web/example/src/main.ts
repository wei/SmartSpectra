import { SmartSpectraWebSDK, SmartSpectraMode, MetricsBuffer } from 'smartspectra-web-sdk';

const video = document.getElementById('video') as HTMLVideoElement;
const pulseEl = document.getElementById('pulse')!;
const breathEl = document.getElementById('breath')!;
const blinkEl = document.getElementById('blink')!;
const startBtn = document.getElementById('start') as HTMLButtonElement;

const sdk = SmartSpectraWebSDK.getInstance();

async function init() {
  const stream = await navigator.mediaDevices.getUserMedia({ video: true });
  video.srcObject = stream;
  await video.play();
  await sdk.setVideoElement(video);
  await sdk.configure({
    apiKey: 'demo-key',
    serverUrl: (import.meta as any).env.VITE_SERVER_URL || 'http://localhost:8080',
    mode: SmartSpectraMode.CONTINUOUS
  });
}

sdk.onResult = (result: MetricsBuffer) => {
  const pulse = result.pulse?.rate[0]?.value?.toFixed(1);
  const breath = result.breathing?.rate[0]?.value?.toFixed(1);
  const blinks = result.face?.blinkCount;
  if (pulse) pulseEl.textContent = pulse;
  if (breath) breathEl.textContent = breath;
  if (blinks !== undefined) blinkEl.textContent = blinks.toString();
};

startBtn.onclick = () => {
  sdk.start({ mode: SmartSpectraMode.CONTINUOUS });
};

init();
