/**
 * SmartSpectra Web Client Application
 * Handles webcam capture, session management, and real-time metrics display
 */

class SmartSpectraClient {
    constructor() {
        this.apiBaseUrl = 'http://localhost:8080';
        this.websocket = null;
        this.sessionId = null;
        this.isRunning = false;
        this.frameInterval = null;
        this.spotTimeout = null;
        
        // DOM elements
        this.elements = {
            webcam: document.getElementById('webcam'),
            canvas: document.getElementById('canvas'),
            startButton: document.getElementById('startButton'),
            stopButton: document.getElementById('stopButton'),
            status: document.getElementById('status'),
            pulse: document.getElementById('pulse'),
            breaths: document.getElementById('breaths'),
            blinkIndicator: document.getElementById('blink-indicator'),
            log: document.getElementById('log'),
            resolution: document.getElementById('resolution'),
            modeContinuous: document.getElementById('modeContinuous'),
            modeSpot: document.getElementById('modeSpot')
        };
        
        this.initializeEventListeners();
        this.initializeWebcam();
    }
    
    initializeEventListeners() {
        this.elements.startButton.addEventListener('click', () => this.startAnalysis());
        this.elements.stopButton.addEventListener('click', () => this.stopAnalysis());
        
        // Handle page unload
        window.addEventListener('beforeunload', () => {
            if (this.isRunning) {
                this.stopAnalysis();
            }
        });
    }
    
    async initializeWebcam() {
        try {
            this.log('Requesting webcam access...', 'info');
            
            const stream = await navigator.mediaDevices.getUserMedia({
                video: {
                    width: { ideal: 1280 },
                    height: { ideal: 720 },
                    frameRate: { ideal: 30 }
                },
                audio: false
            });
            
            this.elements.webcam.srcObject = stream;
            this.log('Webcam initialized successfully', 'info');
            
        } catch (error) {
            this.log(`Failed to access webcam: ${error.message}`, 'error');
            this.updateStatus('Webcam access denied', 'disconnected');
        }
    }
    
    async startAnalysis() {
        try {
            this.log('Starting analysis session...', 'info');
            this.updateStatus('Creating session...', 'connecting');
            
            // Get selected configuration
            const resolution = this.elements.resolution.value;
            const mode = this.elements.modeContinuous.checked ? 'continuous' : 'spot';
            
            // Create session
            const sessionResponse = await this.createSession(resolution);
            this.sessionId = sessionResponse.session_id;
            
            this.log(`Session created: ${this.sessionId}`, 'info');
            this.log(`Mode: ${mode}, Resolution: ${resolution}`, 'info');
            
            // Connect WebSocket
            await this.connectWebSocket(sessionResponse.stream_url);
            
            // Start sending frames
            this.startFrameCapture();
            
            // Handle spot mode timeout
            if (mode === 'spot') {
                this.spotTimeout = setTimeout(() => {
                    this.log('Spot analysis completed (30 seconds)', 'info');
                    this.stopAnalysis();
                }, 30000);
            }
            
            // Update UI
            this.isRunning = true;
            this.elements.startButton.disabled = true;
            this.elements.stopButton.disabled = false;
            this.updateStatus('Analysis running', 'connected');
            
        } catch (error) {
            this.log(`Failed to start analysis: ${error.message}`, 'error');
            this.updateStatus('Failed to start', 'disconnected');
            this.cleanup();
        }
    }
    
    async stopAnalysis() {
        try {
            this.log('Stopping analysis...', 'info');
            this.updateStatus('Stopping...', 'connecting');
            
            this.cleanup();
            
            if (this.sessionId) {
                await this.deleteSession(this.sessionId);
                this.log(`Session ${this.sessionId} deleted`, 'info');
                this.sessionId = null;
            }
            
            this.updateStatus('Stopped', 'disconnected');
            this.log('Analysis stopped', 'info');
            
        } catch (error) {
            this.log(`Error stopping analysis: ${error.message}`, 'error');
            this.updateStatus('Error stopping', 'disconnected');
        }
    }
    
    async createSession(resolution) {
        const response = await fetch(`${this.apiBaseUrl}/sessions`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                config: {
                    resolution: resolution
                }
            })
        });
        
        if (!response.ok) {
            const error = await response.json();
            throw new Error(error.error || `HTTP ${response.status}`);
        }
        
        return await response.json();
    }
    
    async deleteSession(sessionId) {
        const response = await fetch(`${this.apiBaseUrl}/sessions/${sessionId}`, {
            method: 'DELETE'
        });
        
        if (!response.ok && response.status !== 404) {
            const error = await response.json();
            throw new Error(error.error || `HTTP ${response.status}`);
        }
    }
    
    connectWebSocket(streamUrl) {
        return new Promise((resolve, reject) => {
            this.websocket = new WebSocket(streamUrl);
            this.websocket.binaryType = 'blob';
            
            this.websocket.onopen = () => {
                this.log('WebSocket connected', 'info');
                resolve();
            };
            
            this.websocket.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    this.handleMetrics(data);
                } catch (error) {
                    this.log(`Error parsing metrics: ${error.message}`, 'error');
                }
            };
            
            this.websocket.onclose = (event) => {
                this.log(`WebSocket closed: ${event.reason}`, 'info');
                if (this.isRunning) {
                    this.cleanup();
                    this.updateStatus('Connection lost', 'disconnected');
                }
            };
            
            this.websocket.onerror = (error) => {
                this.log('WebSocket error', 'error');
                reject(new Error('WebSocket connection failed'));
            };
            
            // Timeout for connection
            setTimeout(() => {
                if (this.websocket.readyState !== WebSocket.OPEN) {
                    reject(new Error('WebSocket connection timeout'));
                }
            }, 10000);
        });
    }
    
    startFrameCapture() {
        const canvas = this.elements.canvas;
        const context = canvas.getContext('2d');
        const video = this.elements.webcam;
        
        // Set canvas size to match video
        canvas.width = video.videoWidth || 640;
        canvas.height = video.videoHeight || 480;
        
        this.frameInterval = setInterval(() => {
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                // Draw video frame to canvas
                context.drawImage(video, 0, 0, canvas.width, canvas.height);
                
                // Convert to JPEG blob and send
                canvas.toBlob((blob) => {
                    if (blob && this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                        this.websocket.send(blob);
                    }
                }, 'image/jpeg', 0.8);
            }
        }, 100); // 10 FPS
    }
    
    handleMetrics(data) {
        if (data.type === 'metrics' && data.metrics) {
            const metrics = data.metrics;
            
            // Update heart rate
            if (metrics.heart_rate_bpm !== undefined) {
                this.elements.pulse.textContent = metrics.heart_rate_bpm.toFixed(1);
            }
            
            // Update respiratory rate
            if (metrics.respiratory_rate_rpm !== undefined) {
                this.elements.breaths.textContent = metrics.respiratory_rate_rpm.toFixed(1);
            }
            
            // Handle blink detection
            if (metrics.blink_detected) {
                this.elements.blinkIndicator.classList.add('blink');
                setTimeout(() => {
                    this.elements.blinkIndicator.classList.remove('blink');
                }, 200);
            }
            
            this.log(`Metrics: HR=${metrics.heart_rate_bpm?.toFixed(1) || '--'} BPM, RR=${metrics.respiratory_rate_rpm?.toFixed(1) || '--'} RPM`, 'info');
        }
    }
    
    cleanup() {
        this.isRunning = false;
        
        // Clear intervals and timeouts
        if (this.frameInterval) {
            clearInterval(this.frameInterval);
            this.frameInterval = null;
        }
        
        if (this.spotTimeout) {
            clearTimeout(this.spotTimeout);
            this.spotTimeout = null;
        }
        
        // Close WebSocket
        if (this.websocket) {
            this.websocket.close();
            this.websocket = null;
        }
        
        // Reset UI
        this.elements.startButton.disabled = false;
        this.elements.stopButton.disabled = true;
        this.elements.pulse.textContent = '--';
        this.elements.breaths.textContent = '--';
    }
    
    updateStatus(message, type) {
        this.elements.status.textContent = message;
        this.elements.status.className = `status ${type}`;
    }
    
    log(message, type = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${type}`;
        logEntry.textContent = `[${timestamp}] ${message}`;
        
        this.elements.log.appendChild(logEntry);
        this.elements.log.scrollTop = this.elements.log.scrollHeight;
        
        // Keep only last 100 log entries
        while (this.elements.log.children.length > 100) {
            this.elements.log.removeChild(this.elements.log.firstChild);
        }
        
        console.log(`[SmartSpectra] ${message}`);
    }
}

// Initialize the application when the page loads
document.addEventListener('DOMContentLoaded', () => {
    new SmartSpectraClient();
});
