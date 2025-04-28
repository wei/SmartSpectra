package com.presagetech.smartspectra

import android.content.Context
import android.os.Build
import androidx.camera.core.CameraSelector
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.presage.physiology.proto.MetricsProto.MetricsBuffer
import org.opencv.android.OpenCVLoader
import timber.log.Timber


class SmartSpectraSdk private constructor(private val appContext: Context) {
    private lateinit var apiKey: String

    private val _denseMeshPoints = MutableLiveData<List<Pair<Int, Int>>>()
    val denseMeshPoints: LiveData<List<Pair<Int, Int>>> = _denseMeshPoints

    private val _metricsBuffer = MutableLiveData<MetricsBuffer>()
    val metricsBuffer: LiveData<MetricsBuffer> = _metricsBuffer

    private val _errorMessage = MutableLiveData<String>()
    internal val errorMessage: LiveData<String> = _errorMessage

    init {
        // Uncomment to use test server
        // use with extreme caution
        // useTestServer()
    }

    internal fun getApiKey(): String {
        if (!::apiKey.isInitialized) {
            throw IllegalStateException("API key is not initialized. Use .setApiKey() method on SmartSpectraButton to set the key")
        }
        return apiKey
    }

    fun setApiKey(apiKey: String) {
        this.apiKey = apiKey
    }

    fun setDenseMeshPoints(points: ShortArray) {
        val unflattenedPoints = ArrayList<Pair<Int, Int>>(points.size / 2)
        for (i in points.indices step 2) {
            unflattenedPoints.add(Pair(points[i].toInt(), points[i + 1].toInt()))
        }
        _denseMeshPoints.postValue(unflattenedPoints)
    }

    fun setMeshPointsObserver(observer: (List<Pair<Int, Int>>) -> Unit) {
        denseMeshPoints.observeForever(observer)
    }

    fun setMetricsBuffer(metricsBuffer: MetricsBuffer) {
        _metricsBuffer.postValue(metricsBuffer)
    }

    fun setMetricsBufferObserver(observer: (MetricsBuffer) -> Unit) {
        metricsBuffer.observeForever(observer)
    }

    internal fun setErrorMessage(message: String) {
        _errorMessage.postValue(message)
    }

    // Setter methods to allow users to configure SmartSpectraButton and SmartSpectraView
    fun setMeasurementDuration(measurementDuration: Double) {
        SmartSpectraSdkConfig.spotDuration = measurementDuration
    }

    fun setShowFps(showFps: Boolean) {
        SmartSpectraSdkConfig.SHOW_FPS = showFps
    }

    fun setRecordingDelay(recordingDelay: Int) {
        SmartSpectraSdkConfig.recordingDelay = recordingDelay
    }

    fun setSmartSpectraMode(smartSpectraMode: SmartSpectraMode) {
        SmartSpectraSdkConfig.smartSpectraMode = smartSpectraMode
    }

    fun setCameraPosition(@CameraSelector.LensFacing cameraPosition: Int) {
        SmartSpectraSdkConfig.cameraPosition = cameraPosition
    }

    fun showControlsInScreeningView(customizationEnabled: Boolean) {
        SmartSpectraSdkConfig.showControlsInScreeningView = customizationEnabled
        Timber.e("show controls: ${customizationEnabled}")
    }

    /**
     * This method switches the SDK to use the test server instead of the production server.
     *
     * This is an experimental feature and should only be used for testing purposes.
     *          Ensure you understand the implications before using this method.
     */
    @ExperimentalFeature
    internal fun useTestServer() {
        Timber.w("Using test server. Do not use this for production")
        nativeUseTestServer()
    }

    internal fun clearMetricsBuffer() {
        setMetricsBuffer(MetricsBuffer.getDefaultInstance())
    }

    private external fun nativeUseTestServer()

    companion object {
        @Volatile
        private var INSTANCE: SmartSpectraSdk? = null

        fun initialize(context: Context) {
            if (INSTANCE == null) {
                synchronized(this) {
                    if (INSTANCE == null) {
                        // load support libraries
                        if (isSupportedAbi()) {
                            // Load necessary libraries
                            System.loadLibrary("mediapipe_jni")
                            if (OpenCVLoader.initLocal()) {
                                Timber.i("OpenCV loaded successfully");
                            } else {
                                Timber.e("OpenCV initialization failed!");
                            }
                        }
                        // create a new instance
                        INSTANCE = SmartSpectraSdk(context.applicationContext)
                    }
                }
            }
        }

        fun getInstance(): SmartSpectraSdk {
            return INSTANCE
                ?: throw IllegalStateException("SmartSpectraSdk must be initialized first.")
        }

        internal fun isSupportedAbi(): Boolean {
            Build.SUPPORTED_ABIS.forEach {
                if (it == "arm64-v8a" || it == "armeabi-v7a") {
                    return true
                }
            }
            return false
        }
    }
}
