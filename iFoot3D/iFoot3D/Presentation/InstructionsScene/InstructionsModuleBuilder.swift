//
//  InstructionsModuleBuilder.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 05.01.2023.
//

import UIKit
import Combine

enum InstructionsTransition: Transition {
    case capture(outputs: [CaptureProcessedOutput])
}

final class InstructionsModuleBuilder {
    class func build(
        container: AppContainer,
        outputs: [CaptureProcessedOutput]
    ) -> Module<InstructionsTransition, UIViewController> {
        let viewModel = InstructionsViewModel(outputs: outputs)
        let viewController = InstructionsViewController(viewModel: viewModel)
        return Module(viewController: viewController, transitionPublisher: viewModel.transitionPublisher)
    }
}
