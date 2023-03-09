//
//  ReconstructionServiceWrapper.h
//  iFoot3D
//
//  Created by Illia Khrypunov on 21.02.2023.
//

#ifndef ReconstructionServiceWrapper_h
#define ReconstructionServiceWrapper_h

#import <UIKit/UIKit.h>

@interface ReconstructionServiceWrapper : NSObject
// MARK: - Init
- (nonnull instancetype) init;

// MARK: - Reconstruction
- (bool) reconstruct: (nonnull NSArray<NSArray<NSString *> *> *) rightSidePaths
       leftSidePaths: (nonnull NSArray<NSArray<NSString *> *> *) leftSide
           solePaths: (nonnull NSArray<NSArray<NSString *> *> *) sole
          outputPath: (nonnull NSString *) output;

- (bool) reconstruct: (nonnull NSString *) legPath
           solePaths: (nonnull NSArray<NSArray<NSString *> *> *) sole
        logsFilePath: (nonnull NSString *) logsPath
    outputFolderPath: (nonnull NSString *) logsFolder;

@end

#endif /* ReconstructionServiceWrapper_h */
