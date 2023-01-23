//
//  Module.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import UIKit
import Combine

protocol Transition {}

struct Module<T: Transition, V: UIViewController> {
    let viewController: V
    let transitionPublisher: AnyPublisher<T, Never>
}
