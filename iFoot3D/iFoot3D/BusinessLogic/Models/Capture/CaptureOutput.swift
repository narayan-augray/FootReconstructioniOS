//
//  CaptureOutput.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit
import ARKit

struct CaptureOutput {
    let originalPixelBuffer: CVPixelBuffer
    let depthPixelBuffer: CVPixelBuffer
    let intrinsics: simd_float3x3
    let transform: simd_float4x4 
}
