// Video Player Controller
class VideoPlayerController {
    constructor() {
        this.video = document.getElementById('video');
        this.qualityButtons = document.querySelectorAll('.quality-btn');
        this.init();
    }

    init() {
        this.setupHLS();
        this.setupQualityControls();
    }

    setupHLS() {
        if (Hls.isSupported()) {
            const hls = new Hls();
            hls.loadSource('http://localhost:8080/media/hls_output/master.m3u8');
            hls.attachMedia(this.video);
            hls.on(Hls.Events.MANIFEST_PARSED, () => {
                this.video.play().catch(e => console.log('Auto-play prevented:', e));
            });
        } else if (this.video.canPlayType('application/vnd.apple.mpegurl')) {
            this.video.src = 'http://localhost:8080/media/hls_output/master.m3u8';
        }
    }

    setupQualityControls() {
        this.qualityButtons.forEach(button => {
            button.addEventListener('click', (e) => {
                this.handleQualityChange(e.target);
            });
        });
    }

    handleQualityChange(button) {
        this.qualityButtons.forEach(btn => {
            btn.classList.remove('bg-cyan-600');
            btn.classList.add('bg-gray-700');
        });
        button.classList.remove('bg-gray-700');
        button.classList.add('bg-cyan-600');
    }
}

// Metrics Updater
class MetricsUpdater {
    constructor() {
        this.metricElements = document.querySelectorAll('.metric-value');
        this.init();
    }

    init() {
        this.startMetricsUpdate();
    }

    startMetricsUpdate() {
        // Initial update
        this.updateMetrics();
        
        // Set up periodic updates
        setInterval(() => {
            this.updateMetrics();
        }, 3000);
    }

    updateMetrics() {
        const throughput = (3.5 + Math.random() * 1.5).toFixed(1);
        const latency = Math.floor(100 + Math.random() * 50);
        const errorRate = (Math.random() * 0.5).toFixed(1);
        const clients = Math.floor(40 + Math.random() * 5);

        this.updateMetricDisplays(throughput, latency, errorRate, clients);
        this.updatePerformanceMetrics();
    }

    updateMetricDisplays(throughput, latency, errorRate, clients) {
        this.metricElements[0].textContent = `${throughput} Mbps`;
        this.metricElements[1].textContent = `${latency} ms`;
        this.metricElements[2].textContent = `${errorRate} %`;
        this.metricElements[3].textContent = clients;
    }

    updatePerformanceMetrics() {
        // Simulate updating performance metrics
        const cpuValue = (20 + Math.random() * 15).toFixed(1);
        const memoryValue = (1 + Math.random() * 0.5).toFixed(1);
        const networkValue = Math.floor(70 + Math.random() * 30);
        const bufferValue = (10 + Math.random() * 5).toFixed(1);
        
        // Update the performance metric values
        document.querySelectorAll('.performance-metrics .metric-value')[0].textContent = `${cpuValue}%`;
        document.querySelectorAll('.performance-metrics .metric-value')[1].textContent = `${memoryValue}/4 GB`;
        document.querySelectorAll('.performance-metrics .metric-value')[2].textContent = `${networkValue} Mbps`;
        document.querySelectorAll('.performance-metrics .metric-value')[3].textContent = `${bufferValue}s`;
        
        // Update progress bars
        document.querySelectorAll('.performance-metrics .bg-cyan-500')[0].style.width = `${cpuValue}%`;
        document.querySelectorAll('.performance-metrics .bg-cyan-500')[1].style.width = `${memoryValue/4*100}%`;
        document.querySelectorAll('.performance-metrics .bg-cyan-500')[2].style.width = `${networkValue/200*100}%`;
        document.querySelectorAll('.performance-metrics .bg-cyan-500')[3].style.width = `${bufferValue/20*100}%`;
    }
}

// Main Application
class App {
    constructor() {
        this.init();
    }

    init() {
        this.videoController = new VideoPlayerController();
        this.metricsUpdater = new MetricsUpdater();
        console.log('Minimal dashboard initialized');
    }
}

// Initialize the application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new App();
});
