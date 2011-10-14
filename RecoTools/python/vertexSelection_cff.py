import FWCore.ParameterSet.Config as cms

selectPrimaryVerticesQuality = cms.EDFilter(
    "VertexSelector",
    src = cms.InputTag('offlinePrimaryVerticesWithBS'),
    cut = cms.string("isValid & ndof >= 7 && abs(z) < 24 && position.Rho < 2.0"),
    filter = cms.bool(False),
)

selectedPrimaryVertex = cms.EDFilter(
    "PATSingleVertexSelector",
    mode = cms.string('firstVertex'),
    vertices = cms.InputTag('selectPrimaryVerticesQuality'),
    filter = cms.bool(True)
)

selectPrimaryVertices = cms.Sequence(
    selectPrimaryVerticesQuality + selectedPrimaryVertex)
