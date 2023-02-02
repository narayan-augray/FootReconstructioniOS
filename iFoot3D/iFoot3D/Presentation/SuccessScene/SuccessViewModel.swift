//
//  SuccessViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import Combine

final class SuccessViewModel: BaseViewModel {
    // MARK: - Properties
    let outputs: [CaptureProcessedOutput]
    
    // MARK: - Publishers
    private(set) lazy var transitionPublisher = transitionSubject.eraseToAnyPublisher()
    private let transitionSubject = PassthroughSubject<SuccessTransition, Never>()
    
    // MARK: - Init
    init(outputs: [CaptureProcessedOutput]) {
        self.outputs = outputs
        super.init()
    }
    
    // MARK: - Navigation
    func navigateBack() {
        transitionSubject.send(.back)
    }
}
