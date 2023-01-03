//
//  CaptureServiceEvent.swift
//  iFoot3D
//
//  Created by Illia Khrypunov on 03.01.2023.
//

import UIKit

enum CaptureServiceEvent {
    case capturePositions(positions: [CapturePosition])
    case captureOutput(output: CaptureOutput)
}
