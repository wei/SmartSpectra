#include <napi.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <google/protobuf/util/json_util.h>
#include <physiology/modules/messages/status.pb.h>

#include "../../cpp/smartspectra/container/background_container.hpp"
#include "../../cpp/smartspectra/container/settings.hpp"

using namespace presage::smartspectra::container;
namespace phy = physiology;

class ContainerWrapper : public Napi::ObjectWrap<ContainerWrapper> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "Container", {
      InstanceMethod("initialize", &ContainerWrapper::Initialize),
      InstanceMethod("start", &ContainerWrapper::Start),
      InstanceMethod("stop", &ContainerWrapper::Stop),
      InstanceMethod("addFrame", &ContainerWrapper::AddFrame)
    });
    exports.Set("Container", func);
    return exports;
  }

  ContainerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<ContainerWrapper>(info) {}

  Napi::Value Initialize(const Napi::CallbackInfo& info) {
    std::string apiKey = info[0].As<Napi::String>();
    // config object is ignored for now
    Napi::Function metricsCb = info[2].As<Napi::Function>();
    Napi::Function statusCb = info[3].As<Napi::Function>();
    metricsTsfn_ = Napi::ThreadSafeFunction::New(info.Env(), metricsCb, "metrics", 0, 1);
    statusTsfn_ = Napi::ThreadSafeFunction::New(info.Env(), statusCb, "status", 0, 1);

    settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Rest> settings;
    settings.rest().api_key = apiKey;
    container_ = std::make_unique<CpuSpotRestBackgroundContainer>(settings);

    container_->SetOnCoreMetricsOutput([this](const phy::MetricsBuffer& buffer, int64_t ts) {
      std::string json;
      google::protobuf::util::MessageToJsonString(buffer, &json);
      metricsTsfn_.BlockingCall([json, ts](Napi::Env env, Napi::Function cb) {
        cb.Call({ Napi::String::New(env, json), Napi::Number::New(env, ts) });
      });
      return absl::OkStatus();
    });

    container_->SetOnStatusChange([this](phy::StatusCode code) {
      std::string name = phy::StatusCode_Name(code);
      statusTsfn_.BlockingCall([name](Napi::Env env, Napi::Function cb) {
        cb.Call({ Napi::String::New(env, name) });
      });
      return absl::OkStatus();
    });

    container_->Initialize();
    return info.Env().Undefined();
  }

  Napi::Value Start(const Napi::CallbackInfo& info) {
    container_->StartGraph();
    container_->SetRecording(true);
    return info.Env().Undefined();
  }

  Napi::Value Stop(const Napi::CallbackInfo& info) {
    container_->SetRecording(false);
    container_->StopGraph();
    metricsTsfn_.Release();
    statusTsfn_.Release();
    return info.Env().Undefined();
  }

  Napi::Value AddFrame(const Napi::CallbackInfo& info) {
    auto buf = info[0].As<Napi::Buffer<uint8_t>>();
    int64_t timestamp = info[1].As<Napi::Number>().Int64Value();
    std::vector<uint8_t> data(buf.Data(), buf.Data() + buf.Length());
    cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);
    if (!img.empty()) {
      cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
      container_->AddFrameWithTimestamp(img, timestamp * 1000);
    }
    return info.Env().Undefined();
  }

private:
  std::unique_ptr<CpuSpotRestBackgroundContainer> container_;
  Napi::ThreadSafeFunction metricsTsfn_;
  Napi::ThreadSafeFunction statusTsfn_;
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return ContainerWrapper::Init(env, exports);
}

NODE_API_MODULE(smartspectra, InitAll)
