//
//  DataSetManager.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 07.03.2023.
//

import Foundation

protocol DataSetManager: AnyObject {
    func generateOutputs() -> [CaptureProcessedOutput]
}

final class DataSetManagerImpl: DataSetManager {
    func generateOutputs() -> [CaptureProcessedOutput] {
        var outputs: [CaptureProcessedOutput] = []
        
        for index in 0..<CaptureConstants.requiredFootImagesCount {
            if let output = createOutput(index: index, identifier: "\(index)") {
                outputs.append(output)
            }
        }
        
        for identifier in Constant.soleIdentifiers {
            let index = OutputConstants.soleCaptureOutputIndex
            if let output = createOutput(index: index, identifier: identifier) {
                outputs.append(output)
            }
        }
        
        return outputs
    }
}

// MARK: - Private
private extension DataSetManagerImpl {
    func createOutput(
        index: Int,
        identifier: String
    ) -> CaptureProcessedOutput? {
        guard
            let frame = Bundle.main.url(forResource: "original_\(identifier)", withExtension: "png"),
            let depth = Bundle.main.url(forResource: "depth_logs_\(identifier)", withExtension: "txt"),
            let calibration = Bundle.main.url(forResource: "depth_calibration_\(identifier)", withExtension: "txt")
        else {
            return nil
        }
        
        return .init(index: index,
                     originalImageUrl: frame,
                     dataTextFileUrl: depth,
                     calibrationTextFileUrl: calibration)
    }
}

// MARK: - Constant
private enum Constant {
    static let frameFileName: String = "original"
    static let depthFileName: String = "depth_logs"
    static let calibrationFileName: String = "depth_calibration"
    static let soleIdentifiers: [String] = [
        "CC891415-9172-4CFC-97A9-27926CAEC6B3",
        "CF4EEE70-E9E3-4CFD-8304-C125EED19A8C",
        "EBB5F629-89A5-44A5-BDAE-96B923AAC630"
    ]
}
