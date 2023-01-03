//
//  CaptureModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

enum CaptureTransition: Transition {
    case success(outputs: [CaptureProcessedOutput])
}

final class CaptureModuleBuilder {
    class func build(container: AppContainer) -> Module<CaptureTransition, UIViewController> {
        let viewModel = CaptureViewModel(arSessionManager: container.arSessionManager,
                                         captureService: container.captureService,
                                         captureOutputManager: container.captureOutputManager)
        let viewController = CaptureViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
