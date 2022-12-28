//
//  ViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Combine

protocol ViewModel {
    func onViewDidLoad()
    func onViewWillAppear()
    func onViewDidAppear()
    func onViewWillDisappear()
    func onViewDidDisappear()
}
