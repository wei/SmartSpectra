//
//  PresagePreprocessing.h
//  PresagePreprocessing Objective C++ Binding Layer
//
//  Credits:
//  Inspired by MPIrisTracker / MPIrisTrackerDelegate code by Yuki Yamato on 2021/05/05.
//

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <AVFoundation/AVFoundation.h>
#ifdef PRESAGE_PREPROCESSING_BUILD
#import "modules/messages/Status.pbobjc.h"
#else
#import "Status.pbobjc.h"
#endif

typedef NS_ENUM(NSInteger, PresageMode) {
    PresageModeSpot,
    PresageModeContinuous
};

typedef NS_ENUM(NSInteger, PresageServer) {
    PresageServerTest,
    PresageServerProd,
    PresageServerBeta
};

@class PresagePreprocessing;

@protocol PresagePreprocessingDelegate <NSObject>

- (void)frameWillUpdate:(PresagePreprocessing *)tracker
   didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer
              timestamp:(long)timestamp;

- (void)frameDidUpdate:(PresagePreprocessing *)tracker
  didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer;

- (void)statusCodeChanged:(PresagePreprocessing *)tracker
               statusCode:(StatusCode)statusCode;

- (void)metricsBufferChanged:(PresagePreprocessing *)tracker
               serializedBytes:(NSData *)data;

- (void)edgeMetricsChanged:(PresagePreprocessing *)tracker
              serializedBytes:(NSData *)data;

- (void)timerChanged:(double)timerValue;

- (void)receiveDenseFacemeshPoints:(NSArray<NSNumber *> *)points;

- (void)handleGraphError:(NSError *)error;

@end

@interface PresagePreprocessing : NSObject

@property(nonatomic, weak) id <PresagePreprocessingDelegate> delegate;
@property(nonatomic, assign) PresageMode mode;
@property(nonatomic, copy) NSString *apiKey;
@property(nonatomic, copy) NSString *graphName;
@property(nonatomic, assign) AVCaptureDevicePosition cameraPosition;
@property(nonatomic, assign) double spotDuration;

+ (void)configureAuthClientWith:(NSDictionary *)plistData;
+ (NSString *)fetchAuthChallenge;
+ (NSString *)respondToAuthChallengeWith:(NSString *)base64EncodedAnswer for:(NSString *)bundleID;
+ (BOOL)isAuthTokenExpired;
+ (void)useTestServer; // TODO: Deprecate this and remove
+ (void)setServer:(PresageServer)server;

- (instancetype)init;
- (void)start;
- (void)stop;
- (void)buttonStateChangedInFramework:(BOOL)isRecording;
- (NSString * _Nonnull)getStatusHint:(StatusCode)statusCode;
- (void)setCameraPosition:(AVCaptureDevicePosition)cameraPosition;
- (void)setMode:(PresageMode)mode;
- (void)setSpotDuration:(double)spotDuration;

@end
