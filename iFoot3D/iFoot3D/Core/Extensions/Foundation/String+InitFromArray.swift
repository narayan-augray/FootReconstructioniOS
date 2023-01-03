//
//  String+InitFromArray.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Foundation

extension String {
    static func initFromArray(array: [[Float32]]) -> String {
        let height = array.count
        let width = array.first?.count ?? 0
        
        var result: String = ""
        
        for y in 1...height - 1 {
            var lineString = ""
            for x in 1...width - 1 {
                lineString += String(array[y][x])
                if x != width - 1 {
                    lineString += ","
                }
            }
            lineString += "\n"
            result += lineString
        }
        
        return result
    }
}
