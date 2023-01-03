//
//  FileManager+FilePath.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Foundation

extension FileManager {
    static func filePath(filename: String) -> URL {
        let documentDirectory = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        return documentDirectory.appendingPathComponent(filename)
    }
}
