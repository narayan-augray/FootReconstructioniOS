//
//  ViewModel.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 28.12.2022.
//

import Combine

protocol ViewModel {
    // MARK: - Publishers
    var errorPublisher: AnyPublisher<String, Never> { get }
    var isLoadingPublisher: AnyPublisher<Bool, Never> { get }
    
    // MARK: - Funcs
    func onViewDidLoad()
    func onViewWillAppear()
    func onViewDidAppear()
    func onViewWillDisappear()
    func onViewDidDisappear()
}
