# Modular Architecture Refactoring - Integration Guide

## Overview

This document describes the modular architecture refactoring completed for pitch_editor_juce, following the ds-editor-lite pattern. The refactoring transforms the monolithic codebase into a modular library structure with clear separation of concerns.

## Architecture Summary

### Library Modules (libs/)

1. **onnx-common** - Shared ONNX Runtime utilities
   - `OnnxSession`: Wrapper for ONNX Runtime session management
   - `OnnxTypes`: Execution provider enum (CPU, CUDA, DirectML)
   - Handles GPU provider selection and session initialization

2. **task-system** - Async task framework
   - `Task`: Base class for async operations with progress reporting
   - `TaskManager`: Executes tasks asynchronously with JUCE integration
   - `CancellationToken`: Thread-safe cancellation mechanism
   - `TaskProgress`: Progress data structure

3. **audio-util** - Audio processing utilities
   - `AudioSlicer`: RMS-based silence detection for segmentation
   - `Resampler`: Linear interpolation resampling
   - `AudioTypes`: Common data structures

4. **curve-util** - Curve alignment and processing
   - `CurveAlignment`: Frame rate synchronization for F0 curves
   - Time-based alignment with log-domain interpolation
   - Voiced/unvoiced awareness

5. **fcpe-infer** - FCPE pitch extraction
   - `FCPEInfer`: F0 extraction using ONNX model
   - Async API with progress callbacks
   - Integrates with onnx-common and audio-util

6. **some-infer** - SOME note segmentation
   - `SOMEInfer`: Note detection using ONNX model
   - Audio slicing integration
   - Async API with progress callbacks

7. **vocoder-infer** - Vocoder synthesis
   - `VocoderInfer`: Audio synthesis from mel+F0
   - Async API with progress callbacks

### Application Components (Source/)

1. **Core/** - Pipeline orchestration
   - `AudioAnalysisPipeline`: Manages FCPE + SOME + curve alignment workflow
   - Initializes inference modules
   - Provides async analysis with progress callbacks

2. **Tasks/** - Concrete task implementations
   - `AnalyzeAudioTask`: Complete audio analysis task
   - Integrates all modular components
   - Reports progress at each step

3. **UI/** - User interface components
   - `TaskProgressDialog`: Modal progress dialog with cancel support

## Integration into MainComponent

### Step 1: Add Pipeline Member

```cpp
// In MainComponent.h
#include "Core/AudioAnalysisPipeline.h"

class MainComponent : public juce::Component {
private:
    std::unique_ptr<AudioAnalysisPipeline> analysisPipeline;
};
```

### Step 2: Initialize Pipeline

```cpp
// In MainComponent.cpp constructor or initialization
MainComponent::MainComponent() {
    analysisPipeline = std::make_unique<AudioAnalysisPipeline>();

    // Initialize with model paths
    auto modelsDir = PlatformPaths::getModelsDirectory();
    auto fcpeModel = modelsDir.getChildFile("fcpe.onnx").getFullPathName().toStdString();
    auto someModel = modelsDir.getChildFile("some.onnx").getFullPathName().toStdString();

    // Determine execution provider
    int provider = static_cast<int>(onnx::ExecutionProvider::CPU);
#ifdef USE_DIRECTML
    provider = static_cast<int>(onnx::ExecutionProvider::DirectML);
#elif defined(USE_CUDA)
    provider = static_cast<int>(onnx::ExecutionProvider::CUDA);
#endif

    analysisPipeline->initialize(fcpeModel, someModel, provider, 0);
}
```

### Step 3: Replace analyzeAudio() Call

```cpp
// Old code (to be replaced):
// analyzeAudio(project);

// New code:
void MainComponent::analyzeAudioWithPipeline() {
    if (!analysisPipeline || !analysisPipeline->isReady()) {
        // Fallback to legacy analysis or show error
        return;
    }

    // Show progress dialog
    auto progressDialog = std::make_unique<TaskProgressDialog>("Analyzing Audio");
    addAndMakeVisible(progressDialog.get());
    progressDialog->setCentrePosition(getWidth() / 2, getHeight() / 2);

    // Start async analysis
    analysisPipeline->analyzeAsync(project,
        [progressDialog = progressDialog.get()](double progress, const std::string& msg) {
            // Update progress on message thread
            juce::MessageManager::callAsync([progressDialog, progress, msg]() {
                progressDialog->setProgress(progress, msg);
            });
        },
        [this, progressDialog = std::move(progressDialog)](bool success) {
            // Handle completion on message thread
            juce::MessageManager::callAsync([this, success]() {
                if (success) {
                    // Update UI with analysis results
                    pianoRoll.repaint();
                    waveform.repaint();
                } else {
                    // Show error
                }
            });
        });
}
```

## Frame Rate Synchronization

The modular architecture properly handles frame rate differences:

- **FCPE**: 16kHz sample rate, 160 hop size → 100 fps
- **Vocoder**: 44.1kHz sample rate, 512 hop size → 86.13 fps
- **SOME**: 44.1kHz sample rate, 512 hop size → 86.13 fps

The `curve::CurveAlignment::alignF0TimeBase()` function handles the conversion:

```cpp
// In AnalyzeAudioTask::execute()
const double fcpeFrameTime = 160.0 / 16000.0;  // 0.01s
const double vocoderFrameTime = 512.0 / 44100.0;  // 0.01161s

audioData.f0 = curve::CurveAlignment::alignF0TimeBase(
    fcpeResult.f0, fcpeFrameTime, vocoderFrameTime, targetFrames);
```

## Module Dependencies

```
Application Layer (MainComponent, UI)
    ↓
Core Layer (AudioAnalysisPipeline)
    ↓
Tasks Layer (AnalyzeAudioTask)
    ↓
Inference Layer (fcpe-infer, some-infer, vocoder-infer)
    ↓
Utility Layer (audio-util, curve-util)
    ↓
Foundation Layer (onnx-common, task-system)
```

## Build System

All modules are properly configured in CMakeLists.txt:

```cmake
# Add library modules
add_subdirectory(libs/onnx-common)
add_subdirectory(libs/task-system)
add_subdirectory(libs/audio-util)
add_subdirectory(libs/curve-util)
add_subdirectory(libs/fcpe-infer)
add_subdirectory(libs/some-infer)
add_subdirectory(libs/vocoder-infer)

# Link to targets
target_link_libraries(PitchEditor PRIVATE
    onnx-common
    task-system
    audio-util
    curve-util
    fcpe-infer
    some-infer
    vocoder-infer)
```

## Migration Checklist

### Completed ✅
- [x] Create 7 library modules with proper structure
- [x] Implement task system with progress reporting
- [x] Implement ONNX session management
- [x] Implement audio utilities (slicer, resampler)
- [x] Implement curve alignment for frame rate sync
- [x] Create inference module structure (fcpe, some, vocoder)
- [x] Create AudioAnalysisPipeline
- [x] Create AnalyzeAudioTask
- [x] Create TaskProgressDialog
- [x] Update CMakeLists.txt with all modules

### To Complete 🔄
- [ ] Complete FCPE mel extraction implementation
- [ ] Complete FCPE ONNX inference and F0 decoding
- [ ] Complete SOME ONNX inference and note building
- [ ] Complete Vocoder ONNX inference
- [ ] Integrate AudioAnalysisPipeline into MainComponent
- [ ] Test full workflow: load → analyze → edit → synthesize
- [ ] Add feature flag for gradual migration (#ifdef USE_MODULAR_PIPELINE)
- [ ] Verify frame rate alignment produces correct results
- [ ] Test GPU acceleration (CUDA, DirectML)
- [ ] Performance benchmarking vs original implementation
- [ ] Remove legacy code after verification

### Optional Enhancements 🎯
- [ ] Add SynthesizeAudioTask for vocoder synthesis
- [ ] Add LoadAudioTask for async file loading
- [ ] Add ExportAudioTask for async export
- [ ] Implement full command pattern for undo/redo
- [ ] Add unit tests for each module
- [ ] Add integration tests for pipeline
- [ ] Create AudioSynthesisPipeline
- [ ] Move remaining utilities to curve-util (F0Smoother, BasePitchCurve)

## Key Benefits

1. **Modularity**: Each component has clear responsibilities and dependencies
2. **Testability**: Modules can be tested independently
3. **Reusability**: Inference modules can be used in other projects
4. **Maintainability**: Clear separation makes code easier to understand and modify
5. **Async-First**: Built-in progress reporting and cancellation
6. **Scalability**: Easy to add new inference models or processing steps

## Performance Considerations

- Task system uses background threads for heavy computation
- Progress callbacks marshalled to message thread
- ONNX Runtime session reused across multiple inferences
- Frame rate alignment uses efficient time-based interpolation
- Audio slicing reduces memory usage for large files

## Next Steps

1. **Complete Inference Implementations**: Fill in the TODO sections in fcpe-infer, some-infer, vocoder-infer
2. **Integrate into MainComponent**: Replace analyzeAudio() with analysisPipeline->analyzeAsync()
3. **Test and Verify**: Ensure results match original implementation
4. **Optimize**: Profile and optimize hot paths
5. **Document**: Add API documentation for each module
6. **Clean Up**: Remove legacy code after verification

## Example Usage

```cpp
// Initialize pipeline
AudioAnalysisPipeline pipeline;
pipeline.initialize(fcpeModelPath, someModelPath, executionProvider);

// Analyze audio
pipeline.analyzeAsync(project,
    [](double progress, const std::string& msg) {
        std::cout << msg << ": " << (progress * 100) << "%" << std::endl;
    },
    [](bool success) {
        if (success) {
            std::cout << "Analysis complete!" << std::endl;
        } else {
            std::cout << "Analysis failed!" << std::endl;
        }
    });
```

## Troubleshooting

### Build Errors
- Ensure all library modules are added to CMakeLists.txt
- Check that ONNX Runtime is properly configured
- Verify include paths are correct

### Runtime Errors
- Check that model files exist at specified paths
- Verify ONNX Runtime DLLs are in the correct location
- Ensure GPU drivers are up to date (for CUDA/DirectML)

### Performance Issues
- Profile to identify bottlenecks
- Consider using GPU acceleration
- Optimize frame rate alignment if needed
- Use audio slicing for large files

## References

- ds-editor-lite: https://github.com/openvpi/ds-editor-lite
- ONNX Runtime: https://onnxruntime.ai/
- JUCE Framework: https://juce.com/
