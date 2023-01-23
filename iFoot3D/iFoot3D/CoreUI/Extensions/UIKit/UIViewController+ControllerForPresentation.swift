//
//  UIViewController+ControllerForPresentation.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit

extension UIViewController {
    var controllerForPresentation: UIViewController {
        var controllerToPresent: UIViewController = self
        var currentController = presentedViewController
        while currentController != nil {
            if let currentController = currentController, currentController.presentedViewController == nil {
                controllerToPresent = currentController
                break
            } else {
                currentController = currentController?.presentedViewController
            }
        }
        return controllerToPresent
    }
}
