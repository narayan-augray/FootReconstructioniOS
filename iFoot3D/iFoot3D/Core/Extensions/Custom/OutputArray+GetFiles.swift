//
//  OutputArray+GetFiles.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Foundation

extension Array where Element == CaptureProcessedOutput {
    func getFilesUrls() -> [URL?] {
        var fileUrls: [URL?] = []
        
        for output in self {
            fileUrls.append(contentsOf: output.getFiles())
        }
        
        return fileUrls
    }
}
