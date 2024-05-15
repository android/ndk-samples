package com.android.ndk.samples.vectorization

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.snapshots.SnapshotStateMap
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.android.ndk.samples.vectorization.ui.theme.NDKSamplesTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlin.time.Duration
import kotlin.time.Duration.Companion.nanoseconds

// Keep in sync with the definition in benchmark.h.
enum class Backend(val id: Int, val label: String) {
    AUTO_VECTORIZATION(0, "Auto-vectorization"),
    CXX_SIMD(1, "std::simd"),
    CLANG_VECTORS(2, "Clang Vectors"),
    CLANG_MATRICES(3, "Clang Matrices"),
    OPEN_MP(4, "OpenMP"),
}

class VectorizationActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            NDKSamplesTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    Column(
                        modifier = Modifier
                            .padding(innerPadding)
                            .fillMaxWidth()
                    ) {
                        Text(text = "Average times for 10,000,000 runs")
                        BenchmarkTable()
                    }
                }
            }
        }
    }

    init {
        System.loadLibrary("app")
    }
}

sealed interface BenchmarkResult {
    class Success(private val duration: Duration) : BenchmarkResult {
        override fun toString(): String = duration.toString()
    }

    class Failure(private val message: String) : BenchmarkResult {
        override fun toString(): String = message
    }
}

object AppJni {
    fun benchmarkMatrixMultiply(backend: Backend): BenchmarkResult {
        val result = benchmarkMatrixMultiply(backend.id)
        if (result >= 0) {
            return BenchmarkResult.Success(result.nanoseconds)
        }

        return BenchmarkResult.Failure(
            when (result) {
                -1L -> "Not implemented"
                -2L -> "Not supported"
                -3L -> "Invalid backend"
                else -> "Unknown error"
            }
        )
    }

    private external fun benchmarkMatrixMultiply(backend: Int): Long
}

@Composable
fun BenchmarkTable(modifier: Modifier = Modifier) {
    val status: SnapshotStateMap<Backend, String> = remember {
        mutableStateMapOf(*Backend.entries.map { it to "Not started" }
            .toTypedArray())
    }

    LaunchedEffect(true) {
        withContext(Dispatchers.Default) {
            Backend.entries.forEach {
                status[it] = "Running..."
                status[it] = AppJni.benchmarkMatrixMultiply(it).toString()
            }
        }
    }

    Column(
        modifier = modifier
    ) {
        status.toSortedMap().forEach { (backend, status) ->
            BenchmarkResult(name = backend.label, duration = status)
        }
    }
}

@Composable
fun BenchmarkResult(name: String, duration: String) {
    Row(modifier = Modifier.fillMaxWidth()) {
        Text(text = name)
        Spacer(modifier = Modifier.weight(1f))
        Text(text = duration)
    }
}

@Preview(showBackground = true)
@Composable
fun BenchmarkResultPreview() {
    NDKSamplesTheme {
        BenchmarkResult("Auto-vectorization", "00m10s")
    }
}