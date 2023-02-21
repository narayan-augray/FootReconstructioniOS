//
//  PreviewViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import Foundation
import UIKit
import Combine

final class PreviewViewModel: BaseViewModel {
    // MARK: - Properties
    let objectUrl: URL?
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<PreviewTransition, Never>()
    
    // MARK: - Init
    init(modelPath: String) {
        objectUrl = URL(fileURLWithPath: modelPath)
        
        super.init()
    }
    
    // MARK: - Navigation
    func close() {
        transitionSubject.send(.close)
    }
    
    func success() {
        transitionSubject.send(.success)
    }
    
    // MARK: - Helpers
    func deleteFiles() {
        guard let objectUrl = objectUrl else {
            return
        }
        deleteFiles(fileUrls: [objectUrl])
    }
}
