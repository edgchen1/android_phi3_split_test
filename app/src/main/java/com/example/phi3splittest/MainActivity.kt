package com.example.phi3splittest

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.system.Os
import android.util.Log
import com.example.phi3splittest.databinding.ActivityMainBinding
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    private var backgroundExecutor = Executors.newSingleThreadExecutor()

    override fun onCreate(savedInstanceState: Bundle?) {
        val adspLibraryPath = applicationContext.applicationInfo.nativeLibraryDir
        Os.setenv("ADSP_LIBRARY_PATH", adspLibraryPath, true)

        System.loadLibrary("phi3splittest")

        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()

        binding.button.setOnClickListener {
            backgroundExecutor.submit {
                val result = runPhi3()

                runOnUiThread {
                    binding.sampleText.text = result
                }
            }
        }
    }

    override fun onDestroy() {
        backgroundExecutor.shutdown()
        super.onDestroy()
    }

    /**
     * A native method that is implemented by the 'phi3splittest' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    external fun runPhi3(): String

    companion object {
        val tag: String = "phi3splittest"

        // Used to load the 'phi3splittest' library on application startup.
        init {
            Log.i(tag, "MainActivity companion init")

            System.loadLibrary("phi3splittest")
        }
    }
}