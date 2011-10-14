/*
 * Produce a PATFinalStateEvent container with some interesting event info.
 *
 * */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FinalStateAnalysis/DataFormats/interface/PATFinalStateEvent.h"
#include "FinalStateAnalysis/DataFormats/interface/PATFinalStateEventFwd.h"

#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/TriggerEvent.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

class PATFinalStateEventProducer : public edm::EDProducer {
  public:
    PATFinalStateEventProducer(const edm::ParameterSet& pset);
    virtual ~PATFinalStateEventProducer(){}
    void produce(edm::Event& evt, const edm::EventSetup& es);
  private:
    edm::InputTag rhoSrc_;
    edm::InputTag pvSrc_;
    edm::InputTag verticesSrc_;
    edm::InputTag metSrc_;
    edm::InputTag trgSrc_;
    edm::InputTag puInfoSrc_;
    edm::ParameterSet extraWeights_;
};

PATFinalStateEventProducer::PATFinalStateEventProducer(
    const edm::ParameterSet& pset) {
  rhoSrc_ = pset.getParameter<edm::InputTag>("rhoSrc");
  pvSrc_ = pset.getParameter<edm::InputTag>("pvSrc");
  verticesSrc_ = pset.getParameter<edm::InputTag>("verticesSrc");
  metSrc_ = pset.getParameter<edm::InputTag>("metSrc");
  trgSrc_ = pset.getParameter<edm::InputTag>("trgSrc");
  puInfoSrc_ = pset.getParameter<edm::InputTag>("puInfoSrc");
  extraWeights_ = pset.getParameterSet("extraWeights");
  produces<PATFinalStateEventCollection>();
}

void PATFinalStateEventProducer::produce(edm::Event& evt,
    const edm::EventSetup& es) {
  std::auto_ptr<PATFinalStateEventCollection> output(
      new PATFinalStateEventCollection);

  edm::Handle<double> rho;
  evt.getByLabel(rhoSrc_, rho);

  edm::Handle<edm::View<reco::Vertex> > pv;
  evt.getByLabel(pvSrc_, pv);
  edm::Ptr<reco::Vertex> pvPtr = pv->ptrAt(0);

  edm::Handle<edm::View<reco::Vertex> > vertices;
  evt.getByLabel(verticesSrc_, vertices);
  edm::PtrVector<reco::Vertex> verticesPtr = vertices->ptrVector();

  edm::Handle<edm::View<pat::MET> > met;
  evt.getByLabel(metSrc_, met);
  edm::Ptr<pat::MET> metPtr = met->ptrAt(0);

  edm::Handle<pat::TriggerEvent> trig;
  evt.getByLabel(trgSrc_, trig);

  edm::Handle<std::vector<PileupSummaryInfo> > puInfo;
  evt.getByLabel(puInfoSrc_, puInfo);

  // Only get PU info if it exist (i.e. not for data)
  std::vector<PileupSummaryInfo> myPuInfo;
  if (puInfo.isValid())
    myPuInfo = * puInfo;

  PATFinalStateEvent theEvent(*rho, pvPtr, verticesPtr, metPtr,
      *trig, myPuInfo, evt.id());

  std::vector<std::string> extras = extraWeights_.getParameterNames();
  for (size_t i = 0; i < extras.size(); ++i) {
    if (extraWeights_.existsAs<double>(extras[i])) {
      theEvent.addWeight(extras[i],
          extraWeights_.getParameter<double>(extras[i]));
    } else {
      edm::InputTag weightSrc = extraWeights_.getParameter<edm::InputTag>(
          extras[i]);
      edm::Handle<double> weightH;
      evt.getByLabel(weightSrc, weightH);
      theEvent.addWeight(extras[i], *weightH);
    }
  }
  output->push_back(theEvent);
  evt.put(output);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATFinalStateEventProducer);
