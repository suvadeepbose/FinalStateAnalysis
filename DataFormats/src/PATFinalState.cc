#include "FinalStateAnalysis/DataFormats/interface/PATFinalState.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/deltaR.h"
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <algorithm>
#include "TMath.h"

namespace {
  double transverseMass(const reco::Candidate::LorentzVector& p1,
      const reco::Candidate::LorentzVector& p2){
    double totalEt = p1.Et() + p2.Et();
    double totalPt = (p1 + p2).pt();
    return std::sqrt(totalEt*totalEt - totalPt*totalPt);
  }

  // Taken from CommonTools/CandUtils/AddFourMomenta.h
  void addFourMomenta( reco::Candidate & c ) {
    reco::Candidate::LorentzVector p4( 0, 0, 0, 0 );
    reco::Candidate::Charge charge = 0;
    size_t n = c.numberOfDaughters();
    for(size_t i = 0; i < n; ++i) {
      const reco::Candidate * d = (const_cast<const reco::Candidate &>(c)).daughter(i);
      p4 += d->p4();
      charge += d->charge();
    }
    c.setP4( p4 );
    c.setCharge( charge );
  }

  // Predicate to sort a collection of indices, which correspond to a list of
  // candidates, by descending pt
  class CandPtIndexOrdering {
    public:
      CandPtIndexOrdering(const std::vector<const reco::Candidate*>& cands):
        cands_(cands){}
      bool operator()(size_t i1, size_t i2) {
        const reco::Candidate* cand1 = cands_[i1];
        assert(cand1);
        const reco::Candidate* cand2 = cands_[i2];
        assert(cand2);
        return cand1->pt() > cand2->pt();
      }
    private:
      const std::vector<const reco::Candidate*>& cands_;
  };

  class CandPtOrdering {
    public:
      bool operator()(const reco::Candidate* c1, const reco::Candidate* c2) {
        assert(c1); assert(c2);
        return c1->pt() > c2->pt();
      }
  };
}

// empty constructor
PATFinalState::PATFinalState():PATLeafCandidate(){}

PATFinalState::PATFinalState(
    int charge, const reco::Candidate::LorentzVector& p4,
    const edm::Ptr<pat::MET>& met,
    const edm::Ptr<reco::Vertex>& vertex,
    const edm::Ptr<PATFinalStateEvent>& event):PATLeafCandidate(
      reco::LeafCandidate(charge, p4)) {
  met_ = met;
  vertex_ = vertex;
  event_ = event;
  if (vertex_.isNonnull())
    setVertex(vertex_->position());
}

const reco::Candidate* PATFinalState::daughter(size_t i) const {
  const reco::Candidate* output = daughterUnsafe(i);
  if (!output) {
    throw cms::Exception("NullDaughter") <<
      "PATFinalState::daughter(" << i << ") is null!" << std::endl;
  }
  return output;
}

const reco::CandidatePtr PATFinalState::daughterPtr(size_t i) const {
  reco::CandidatePtr output = daughterPtrUnsafe(i);
  if (output.isNull())
    throw cms::Exception("NullDaughter") <<
      "PATFinalState::daughterPtr(" << i << ") is null!" << std::endl;
  return output;
}

const reco::CandidatePtr
PATFinalState::daughterUserCand(size_t i, const std::string& tag) const {
  const reco::CandidatePtr output = daughterUserCandUnsafe(i, tag);
  if (output.isNull())
    throw cms::Exception("NullDaughter") <<
      "PATFinalState::daughterUserCand(" << i << ","
      << tag << ") is null!" << std::endl;
  return output;
}

bool
PATFinalState::daughterHasUserCand(size_t i, const std::string& tag) const {
  reco::CandidatePtr userCand = daughterUserCandUnsafe(i, tag);
  return userCand.isNonnull();
}

const PATFinalState::LorentzVector&
PATFinalState::daughterUserCandP4(size_t i, const std::string& tag) const {
  if (tag == "")
    return daughter(i)->p4();
  reco::CandidatePtr userCand = daughterUserCand(i, tag);
  assert(userCand.isNonnull());
  return userCand->p4();
}

std::vector<const reco::Candidate*> PATFinalState::daughters() const {
  std::vector<const reco::Candidate*> output;
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    output.push_back(daughter(i));
  }
  return output;
}

reco::CandidatePtrVector
PATFinalState::daughterPtrs(const std::string& tags) const {
  std::vector<std::string> tokens;
  tokens.reserve(numberOfDaughters());

  // remove any whitespace
  std::string cleanSysTags = boost::algorithm::erase_all_copy(tags, " ");
  boost::split(tokens, cleanSysTags, boost::is_any_of(","));

  if (tokens.size() != numberOfDaughters()) {
    throw cms::Exception("BadTokens") <<
      "PATFinalState::daughterPtrs(tags) The number of parsed tokens ("
      << tokens.size() << ") from the token string: " << tags
      << " does not match the number of daughters (" << numberOfDaughters()
      << ")" << std::endl;
  }

  reco::CandidatePtrVector output;
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    const std::string& token = tokens[i];
    if (token == "#") // skip daughter
      continue;
    if (token == "" || token == "@") // no sys tag specified
      output.push_back(daughterPtr(i));
    else
      output.push_back(daughterUserCand(i, token));
  }
  return output;
}

std::vector<const reco::Candidate*>
PATFinalState::daughters(const std::string& tags) const {
  if (tags == "")
    return daughters();
  std::vector<std::string> tokens;
  tokens.reserve(numberOfDaughters());

  // remove any whitespace
  std::string cleanSysTags = boost::algorithm::erase_all_copy(tags, " ");
  boost::split(tokens, cleanSysTags, boost::is_any_of(","));

  if (tokens.size() != numberOfDaughters()) {
    throw cms::Exception("BadTokens") <<
      "PATFinalState::daughters(tags) The number of parsed tokens ("
      << tokens.size() << ") from the token string: " << tags
      << " does not match the number of daughters (" << numberOfDaughters()
      << ")" << std::endl;
  }

  std::vector<const reco::Candidate*> output;
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    const std::string& token = tokens[i];
    if (token == "#") // skip daughter
      continue;
    if (token == "" || token == "@") // no sys tag specified
      output.push_back(daughter(i));
    else
      output.push_back(daughterUserCand(i, token).get());
  }
  return output;
}

std::vector<size_t> PATFinalState::indicesByPt(const std::string& tags) const {
  std::vector<const reco::Candidate*> daughtersToSort;
  daughtersToSort.reserve(numberOfDaughters());
  if (tags == "") {
    daughtersToSort = daughters();
  } else {
    daughtersToSort = daughters(tags);
  }
  std::vector<size_t> indices;
  indices.reserve(3);
  indices.push_back(0); indices.push_back(1); indices.push_back(2);

  std::sort(indices.begin(), indices.end(),
      CandPtIndexOrdering(daughtersToSort));
  return indices;
}

std::vector<const reco::Candidate*> PATFinalState::daughtersByPt(
        const std::string& tags) const {
  std::vector<const reco::Candidate*> daughtersToSort;
  daughtersToSort.reserve(numberOfDaughters());
  if (tags == "") {
    daughtersToSort = daughters();
  } else {
    daughtersToSort = daughters(tags);
  }
  std::sort(daughtersToSort.begin(), daughtersToSort.end(), CandPtOrdering());
  return daughtersToSort;
}
const reco::Candidate*
PATFinalState::daughterByPt(size_t i, const std::string& tags) const {
  return daughtersByPt(tags).at(i);
}

bool
PATFinalState::ptOrdered(size_t i, size_t j, const std::string& tags) const {
  std::vector<const reco::Candidate*> d = daughters(tags);
  assert(i < d.size());
  assert(j < d.size());
  return d[i]->pt() > d[j]->pt();
}

bool
PATFinalState::matchToHLTFilter(size_t i, const std::string& filter,
    double maxDeltaR) const {
  const reco::Candidate* dau = this->daughter(i);
  assert(dau);
  return evt()->matchedToFilter(*dau, filter, maxDeltaR);
}

bool
PATFinalState::matchToHLTPath(size_t i, const std::string& path,
    double maxDeltaR) const {
  const reco::Candidate* dau = this->daughter(i);
  assert(dau);
  return evt()->matchedToPath(*dau, path, maxDeltaR);
}

double PATFinalState::eval(const std::string& function) const {
  StringObjectFunction<PATFinalState> functor(function, true);
  return functor(*this);
}

bool PATFinalState::filter(const std::string& cut) const {
  StringCutObjectSelector<PATFinalState> cutter(cut, true);
  return cutter(*this);
}

PATFinalState::LorentzVector
PATFinalState::visP4(const std::string& tags) const {
  LorentzVector output;
  std::vector<const reco::Candidate*> theDaughters = daughters(tags);
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    output += theDaughters[i]->p4();
  }
  return output;
}

PATFinalState::LorentzVector
PATFinalState::visP4() const {
  LorentzVector output;
  std::vector<const reco::Candidate*> theDaughters = daughters();
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    output += theDaughters[i]->p4();
  }
  return output;
}

PATFinalState::LorentzVector PATFinalState::totalP4(
    const std::string& tags, const std::string& metSysTag) const {
  reco::Candidate::LorentzVector output = visP4(tags);
  if (metSysTag != "" && metSysTag != "@") {
    const reco::CandidatePtr& metUserCand = met()->userCand(metSysTag);
    assert(metUserCand.isNonnull());
    output += metUserCand->p4();
  } else {
    output += met()->p4();
  }
  return output;
}

PATFinalState::LorentzVector PATFinalState::totalP4() const {
  return visP4() + met()->p4();
}

double
PATFinalState::dPhi(int i, const std::string& sysTagI,
    int j, const std::string& sysTagJ) const {
  return reco::deltaPhi(daughterUserCandP4(i, sysTagI).phi(),
      daughterUserCandP4(j, sysTagJ).phi());
}

double
PATFinalState::dPhi(int i, int j) const {
  return dPhi(i, "", j, "");
}

double
PATFinalState::smallestDeltaPhi() const {
  double smallestDeltaPhi = 1e9;
  for (size_t i = 0; i < numberOfDaughters()-1; ++i) {
    for (size_t j = i+1; j < numberOfDaughters(); ++j) {
      double deltaPhiIJ = deltaPhi(i, j);
      if (deltaPhiIJ < smallestDeltaPhi) {
        smallestDeltaPhi = deltaPhiIJ;
      }
    }
  }
  return smallestDeltaPhi;
}

double
PATFinalState::dR(int i, const std::string& sysTagI,
    int j, const std::string& sysTagJ) const {
  return reco::deltaR(daughterUserCandP4(i, sysTagI),
      daughterUserCandP4(j, sysTagJ));
}

double
PATFinalState::dR(int i, int j) const {
  return dR(i, "", j, "");
}

double
PATFinalState::smallestDeltaR() const {
  double smallestDeltaR = 1e9;
  for (size_t i = 0; i < numberOfDaughters()-1; ++i) {
    for (size_t j = i+1; j < numberOfDaughters(); ++j) {
      double deltaRIJ = dR(i, j);
      if (deltaRIJ < smallestDeltaR) {
        smallestDeltaR = deltaRIJ;
      }
    }
  }
  return smallestDeltaR;
}

double
PATFinalState::deltaPhiToMEt(int i, const std::string& sysTag,
    const std::string& metTag) const {
  double metPhi = met()->phi();
  if (metTag != "") {
    const reco::CandidatePtr& metUserCand = met()->userCand(metTag);
    assert(metUserCand.isNonnull());
    metPhi = metUserCand->phi();
  }
  return reco::deltaPhi(daughterUserCandP4(i, sysTag).phi(), metPhi);
}

double
PATFinalState::deltaPhiToMEt(int i) const {
  return deltaPhiToMEt(i, "", "");
}

double
PATFinalState::mt(int i, const std::string& sysTagI,
    int j, const std::string& sysTagJ) const {
  return transverseMass(daughterUserCandP4(i, sysTagI),
      daughterUserCandP4(j, sysTagJ));
}

double
PATFinalState::mt(int i, int j) const {
  return mt(i, "", j, "");
}

double PATFinalState::mtMET(int i, const std::string& tag,
    const std::string& metTag) const {
  if (metTag != "") {
    return transverseMass(daughterUserCandP4(i, tag),
        met_->userCand(metTag)->p4());
  } else {
    return transverseMass(daughterUserCandP4(i, tag),
        met_->p4());
  }
}

double PATFinalState::mtMET(int i, const std::string& metTag) const {
  if (metTag != "") {
    return transverseMass(daughterUserCandP4(i, ""),
        met_->userCand(metTag)->p4());
  } else {
    return transverseMass(daughterUserCandP4(i, ""), met_->p4());
  }
}

double PATFinalState::ht(const std::string& sysTags) const {
  std::vector<const reco::Candidate*> theDaughters = daughters(sysTags);
  double output = 0;
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    output += theDaughters[i]->pt();
  }
  return output;
}

double
PATFinalState::ht() const {
  std::vector<const reco::Candidate*> theDaughters = daughters();
  double output = 0;
  for (size_t i = 0; i < numberOfDaughters(); ++i) {
    output += theDaughters[i]->pt();
  }
  return output;
}

reco::CandidatePtrVector PATFinalState::extras(
    const std::string& label, const std::string& filter) const {
  // maybe this needs to be optimized
  if (filter == "")
    return overlaps(label);
  StringCutObjectSelector<reco::Candidate> cut(filter, true);
  const reco::CandidatePtrVector& unfiltered = overlaps(label);
  reco::CandidatePtrVector output;
  for (size_t i = 0; i < unfiltered.size(); ++i) {
    const reco::CandidatePtr& cand = unfiltered[i];
    if (cut(*cand))
      output.push_back(cand);
  }
  return output;
}

reco::CompositePtrCandidate
PATFinalState::subcand(int i, int j, int x, int y, int z) const {
  reco::CompositePtrCandidate output;
  output.addDaughter(daughterPtr(i));
  output.addDaughter(daughterPtr(j));
  if (x > -1)
    output.addDaughter(daughterPtr(x));
  if (y > -1)
    output.addDaughter(daughterPtr(y));
  if (z > -1)
    output.addDaughter(daughterPtr(z));
  addFourMomenta(output);
  return output;
}

reco::CompositePtrCandidate
PATFinalState::subcand(const std::string& tags) const {
  reco::CompositePtrCandidate output;
  const reco::CandidatePtrVector daus = daughterPtrs(tags);
  for (size_t i = 0; i < daus.size(); ++i) {
    output.addDaughter(daus[i]);
  }
  addFourMomenta(output);
  return output;
}

reco::CompositePtrCandidate
PATFinalState::subcand(const std::string& tags,
    const std::string& extraColl, const std::string& filter) const {
  reco::CompositePtrCandidate output;
  const reco::CandidatePtrVector daus = daughterPtrs(tags);
  const reco::CandidatePtrVector cands = extras(extraColl, filter);
  for (size_t i = 0; i < cands.size(); ++i) {
    output.addDaughter(cands[i]);
  }
  for (size_t i = 0; i < daus.size(); ++i) {
    output.addDaughter(daus[i]);
  }
  addFourMomenta(output);
  return output;
}

bool PATFinalState::likeSigned(int i, int j) const {
  return daughter(i)->charge()*daughter(j)->charge() > 0;
}

bool PATFinalState::likeFlavor(int i, int j) const {
  return std::abs(daughter(i)->pdgId()) == std::abs(daughter(j)->pdgId());
}

edm::Ptr<pat::Tau> PATFinalState::daughterAsTau(size_t i) const {
  return daughterAs<pat::Tau>(i);
}
edm::Ptr<pat::Muon> PATFinalState::daughterAsMuon(size_t i) const {
  return daughterAs<pat::Muon>(i);
}
edm::Ptr<pat::Electron> PATFinalState::daughterAsElectron(size_t i) const {
  return daughterAs<pat::Electron>(i);
}
edm::Ptr<pat::Jet> PATFinalState::daughterAsJet(size_t i) const {
  return daughterAs<pat::Jet>(i);
}
