//
//  CaptureProcessedOutput.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Foundation

struct CaptureProcessedOutput {
    let index: Int
    let originalImageUrl: URL
    let dataTextFileUrl: URL
    let calibrationTextFileUrl: URL?
}

// MARK: - Helpers
extension CaptureProcessedOutput {
    func getFiles() -> [URL] {
        var files = [originalImageUrl, dataTextFileUrl]
        if let calibrationTextFileUrl = calibrationTextFileUrl {
            files.append(calibrationTextFileUrl)
        }
        return files
    }
}
