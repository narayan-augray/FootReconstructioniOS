//
//  ReconstructionServiceEvent.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

import Foundation

enum ReconstructionServiceEvent {
    case reconstructed(path: String)
    case failure
}
