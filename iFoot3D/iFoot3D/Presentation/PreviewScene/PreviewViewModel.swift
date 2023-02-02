//
//  PreviewViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 02.02.2023.
//

import Foundation
import Combine

final class PreviewViewModel: BaseViewModel {
    // MARK: - Properties
    let objectUrl: URL
    let outputs: [CaptureProcessedOutput]
    
    // MARK: - Transition
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<PreviewTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput]) {
        self.outputs = outputs
        self.objectUrl = Bundle.main.url(forResource: Object.modelName, withExtension: Object.modelExtension)!
        super.init()
    }
    
    // MARK: - Navigation
    func navigateBack() {
        transitionSubject.send(.back)
    }
}
