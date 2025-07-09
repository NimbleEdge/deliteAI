
Pod::Spec.new do |s|
  # Basic Information
  s.name             = 'DeliteAI'
  s.version          = '0.0.1-local'
  s.summary          = 'On-Device AI platform for creating delightful experiences with agentic workflows.'

  s.description      = <<-DESC
  **DeliteAI** is a powerful **on-device AI platform** designed for developers to build
  **agentic workflows**. It empowers you to deliver secure, privacy-aware, and
  high-performance AI-native experiences and applications across various
  platforms and devices, leveraging the power of local AI inference.
  DESC

  s.homepage         = 'https://github.com/NimbleEdge/deliteAI'
  s.license          = { :type => 'Apache-2.0', :file => '../../LICENSE' }
  s.author           = { 'DeliteAI' => 'team-opensource@nimbleedgehq.ai' }
  s.source           = { :git => 'https://github.com/NimbleEdge/deliteAI-iOS.git', :tag => s.version.to_s }

  # Platform and Swift Version
  s.swift_version    = '5.0'
  s.ios.deployment_target = '12.0'

  # Build Settings for Optimization
  s.pod_target_xcconfig = {
    # Aggressive optimizations for release builds, focusing on binary size and performance.
    'OTHER_CFLAGS'               => '-Oz -fdata-sections -ffunction-sections -flto',
    'OTHER_LDFLAGS'              => '-Wl,--gc-sections -flto',
    'DEAD_CODE_STRIPPING'        => 'YES',
    'ENABLE_BITCODE'             => 'YES',
    'STRIP_INSTALLED_PRODUCT'    => 'YES',
    'DEBUG_INFORMATION_FORMAT'   => 'dwarf-with-dsym',
    'SWIFT_OPTIMIZATION_LEVEL'   => '-Osize',
    'SWIFT_COMPILATION_MODE'     => 'wholemodule'
  }

  # Framework Type
  s.static_framework = true

  # Source Files and Headers
  s.source_files = 'deliteAI/Classes/**/*.{h,m,swift}'
  s.public_header_files = 'deliteAI/Classes/sources/**/*.h', 'deliteAI/Classes/deliteAI.h'

  # Vendored Frameworks (Pre-compiled Binaries)
  # Ensure all paths here are correct relative to your Podspec file.
  s.ios.preserve_paths = 'deliteAI/Assets/*.xcframework'
  s.ios.vendored_frameworks = 'deliteAI/Assets/onnxruntime.xcframework',
                              'deliteAI/Assets/nimblenet.xcframework',
                              'deliteAI/Assets/onnxruntime-genai.xcframework',
                              'deliteAI/Assets/onnxruntime_extensions.xcframework',
                              'deliteAI/Assets/LLaMARunner.xcframework'

  # External Dependencies
  s.dependency 'Alamofire', '~> 5.0'
  s.dependency 'SwiftProtobuf', '~> 1.18'
  s.dependency 'PLCrashReporter'
end
