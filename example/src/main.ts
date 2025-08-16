import { SmartSpectraWebSDK, SmartSpectraMode } from 'smart-spectra-sdk';

const sdk = SmartSpectraWebSDK.getInstance();
const video = document.getElementById('video') as HTMLVideoElement;
const pulseSpan = document.getElementById('pulse')!;
const breathingSpan = document.getElementById('breathing')!;
const blinksSpan = document.getElementById('blinks')!;
let blinkCount = 0;

async function start() {
  const apiKey = (document.getElementById('apiKey') as HTMLInputElement).value;
  const serverUrl = (document.getElementById('serverUrl') as HTMLInputElement).value;
  await sdk.configure({ apiKey, serverUrl, mode: SmartSpectraMode.CONTINUOUS });
  await sdk.setVideoElement(video);
  const stream = await sdk.requestCameraPermission();
  video.srcObject = stream;
  await sdk.start({
    mode: SmartSpectraMode.CONTINUOUS,
    onResult: (res) => {
      const pulseVal = res.pulse?.rate?.[0]?.value;
      const breathVal = res.breathing?.rate?.[0]?.value;
      if (pulseVal) pulseSpan.textContent = pulseVal.toFixed(1);
      if (breathVal) breathingSpan.textContent = breathVal.toFixed(1);
      if (res.face?.blinks) {
        blinkCount += res.face.blinks;
        blinksSpan.textContent = String(blinkCount);
      }
    },
    onError: (err) => console.error(err)
  });
}

async function stop() {
  await sdk.stop();
}

(document.getElementById('startBtn') as HTMLButtonElement).onclick = start;
(document.getElementById('stopBtn') as HTMLButtonElement).onclick = stop;
