import FWCore.ParameterSet.Config as cms

customizeMuonSequence = cms.Sequence()

from FinalStateAnalysis.PatTools.muons.patMuonPFIsoEmbedding_cff import \
        patMuonsLoosePFIsoEmbedded03,\
        patMuonsLoosePFIsoEmbedded04,\
        patMuonsLoosePFIsoEmbedded06,\
        patMuonsLoosePFIsoEmbedded
customizeMuonSequence += patMuonsLoosePFIsoEmbedded

from FinalStateAnalysis.PatTools.muons.patMuonIdEmbedding_cfi import patMuonsEmbedWWId
customizeMuonSequence += patMuonsEmbedWWId

from FinalStateAnalysis.PatTools.muons.patMuonIpEmbedding_cfi import patMuonsEmbedIp
customizeMuonSequence += patMuonsEmbedIp

from FinalStateAnalysis.PatTools.muons.muonSystematics_cfi import \
        poolDBESSourceMuScleFitCentralValue, \
        poolDBESSourceMuScleFitShiftUp, \
        poolDBESSourceMuScleFitShiftDown, \
        patMuonsEmbedSystematics
customizeMuonSequence += patMuonsEmbedSystematics

from FinalStateAnalysis.PatTools.muons.triggerMatch_cfi import triggeredPatMuons
customizeMuonSequence += triggeredPatMuons
