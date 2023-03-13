//
//  FileManager+Folder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.03.2023.
//

import Foundation

extension FileManager {
    static func createFolder(name: String) -> URL {
        let documentDirectory = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        let directoryPath = documentDirectory.appendingPathComponent(name)
        try? FileManager.default.createDirectory(atPath: directoryPath.path, withIntermediateDirectories: false)
        return directoryPath
    }
    
    static func getContents(path: String) -> [URL] {
        let files = try? FileManager.default.contentsOfDirectory(atPath: path)
        return files?.compactMap {
            URL(fileURLWithPath: "\(path)/\($0)")
        } ?? []
    }
}
