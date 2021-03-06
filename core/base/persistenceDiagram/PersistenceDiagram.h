/// \ingroup base
/// \class ttk::PersistenceDiagram
/// \author Guillaume Favelier <guillaume.favelier@lip6.fr>
/// \author Julien Tierny <julien.tierny@lip6.fr>
/// \date September 2016.
///
/// \brief TTK processing package for the computation of persistence diagrams.
///
/// This package computes the persistence diagram of the extremum-saddle pairs 
/// of an input scalar field. The X-coordinate of each pair corresponds to its
/// birth, while its smallest and highest Y-coordinates correspond to its birth
/// and death respectively. 
///
/// In practice, each extremity of a persistence pair is represented by its 
/// vertexId and critical type. Based on that, the persistence of the pair 
/// and its 2D embedding can easily be obtained.
///
/// Persistence diagrams are useful and stable concise representations of the 
/// topological features of a data-set. It is useful to fine-tune persistence 
/// thresholds for topological simplification or for fast similarity 
/// estimations for instance. 
///
/// \b Related \b publication \n
/// "Computational Topology: An Introduction" \n
/// Herbert Edelsbrunner and John Harer \n
/// American Mathematical Society, 2010
///
/// \sa ttkPersistenceDiagram.cpp %for a usage example.

#ifndef _PERSISTENCEDIAGRAM_H
#define _PERSISTENCEDIAGRAM_H

// base code includes
#include<Wrapper.h>
#include<Triangulation.h>
#include<FTMTreePP.h>
#include<MorseSmaleComplex3D.h>

namespace ttk{

  class PersistenceDiagram : public Debug{

    public:

      PersistenceDiagram();
      ~PersistenceDiagram();

      inline int setComputeSaddleConnectors(bool state){
        ComputeSaddleConnectors=state;
        return 0;
      }

      ftm::NodeType getNodeType(ftm::FTMTree_MT* tree,
                                ftm::TreeType    treeType,
                                const SimplexId        vertexId) const;

      template <typename scalarType>
      int sortPersistenceDiagram(std::vector<std::tuple<ftm::idVertex,
                                              ftm::NodeType,
                                              ftm::idVertex,
                                              ftm::NodeType,
                                              scalarType,
                                              ftm::idVertex>>& diagram,
                                              scalarType* scalars,
                                              SimplexId* offsets) const;

      template <typename scalarType>
      int computeCTPersistenceDiagram(
          ftm::FTMTreePP& tree,
          const std::vector<std::tuple<ftm::idVertex, ftm::idVertex, scalarType, bool>>& pairs,
          std::vector<std::tuple<ftm::idVertex,
                       ftm::NodeType,
                       ftm::idVertex,
                       ftm::NodeType,
                       scalarType,
                       ftm::idVertex>>& diagram,
          scalarType*                   scalars) const;

      template <class scalarType>
        int execute() const;

      inline int setDMTPairs(std::vector<std::tuple<dcg::Cell,dcg::Cell>>* data){
        dmt_pairs=data;
        return 0;
      }

      inline int setupTriangulation(Triangulation* data){
        triangulation_ = data;
        if(triangulation_){
          ftm::FTMTreePP contourTree;
          contourTree.setupTriangulation(triangulation_);

          triangulation_->preprocessBoundaryVertices();
        }
        return 0;
      }

      inline int setInputScalars(void* data){
        inputScalars_ = data;
        return 0;
      }

      inline int setInputOffsets(void* data){
        inputOffsets_ = data;
        return 0;
      }

      inline int setOutputCTDiagram(void* data){
        CTDiagram_=data;
        return 0;
      }

    protected:

      std::vector<std::tuple<dcg::Cell,dcg::Cell>>* dmt_pairs;

      bool ComputeSaddleConnectors;

      Triangulation* triangulation_;
      void* inputScalars_;
      void* inputOffsets_;
      void* CTDiagram_;
  };
}

template <typename scalarType>
int ttk::PersistenceDiagram::sortPersistenceDiagram(
    
std::vector<std::tuple<ftm::idVertex,ftm::NodeType,ftm::idVertex,ftm::NodeType,
scalarType,ftm::idVertex>>& diagram,
    scalarType* scalars,
    SimplexId* offsets) const{
  auto cmp=[scalars, offsets](const 
std::tuple<ftm::idVertex,ftm::NodeType,ftm::idVertex,ftm::NodeType,scalarType,
ftm::idVertex>& a,
      const 
std::tuple<ftm::idVertex,ftm::NodeType,ftm::idVertex,ftm::NodeType,scalarType,
ftm::idVertex>& b){
    const ftm::idVertex idA=std::get<0>(a);
    const ftm::idVertex idB=std::get<0>(b);
    const ftm::idVertex va=offsets[idA];
    const ftm::idVertex vb=offsets[idB];
    const scalarType sa=scalars[idA];
    const scalarType sb=scalars[idB];

    if(sa!=sb) return sa < sb;
    else return va < vb;
  };

  std::sort(diagram.begin(), diagram.end(), cmp);

  return 0;
}

template <typename scalarType>
int ttk::PersistenceDiagram::computeCTPersistenceDiagram(
    ftm::FTMTreePP& tree,
    const std::vector<std::tuple<ftm::idVertex, ftm::idVertex, scalarType, 
bool>>& pairs,
    std::vector<std::tuple<ftm::idVertex,
                 ftm::NodeType,
                 ftm::idVertex,
                 ftm::NodeType,
                 scalarType,
                 ftm::idVertex>>& diagram,
    scalarType*                   scalars) const
{
   const ftm::idVertex numberOfPairs = pairs.size();
   diagram.resize(numberOfPairs);
   for (ftm::idVertex i = 0; i < numberOfPairs; ++i) {
      const ftm::idVertex v0               = std::get<0>(pairs[i]);
      const ftm::idVertex v1               = std::get<1>(pairs[i]);
      const scalarType    persistenceValue = std::get<2>(pairs[i]);
      const bool          type             = std::get<3>(pairs[i]);

      std::get<4>(diagram[i]) = persistenceValue;
      if (type == true) {
         std::get<0>(diagram[i]) = v0;
         std::get<1>(diagram[i]) = getNodeType(tree.getJoinTree(), 
ftm::TreeType::Join, v0);
         std::get<2>(diagram[i]) = v1;
         std::get<3>(diagram[i]) = getNodeType(tree.getJoinTree(), 
ftm::TreeType::Join, v1);
         std::get<5>(diagram[i]) = 0;
      } else {
         std::get<0>(diagram[i]) = v1;
         std::get<1>(diagram[i]) = getNodeType(tree.getSplitTree(), 
ftm::TreeType::Split, v1);
         std::get<2>(diagram[i]) = v0;
         std::get<3>(diagram[i]) = getNodeType(tree.getSplitTree(), 
ftm::TreeType::Split, v0);
         std::get<5>(diagram[i]) = 2;
      }
   }

   return 0;
}

template <typename scalarType>
int ttk::PersistenceDiagram::execute() const{
  // get data
  std::vector<std::tuple<ftm::idVertex,ftm::NodeType,ftm::idVertex,ftm::NodeType,scalarType,ftm::idVertex>>& CTDiagram=
    *static_cast<std::vector<std::tuple<ftm::idVertex,ftm::NodeType,ftm::idVertex,ftm::NodeType,scalarType,ftm::idVertex>>*>(CTDiagram_);
  scalarType* scalars=static_cast<scalarType*>(inputScalars_);
  SimplexId* offsets=static_cast<SimplexId*>(inputOffsets_);

  const ftm::idVertex numberOfVertices=triangulation_->getNumberOfVertices();
  // convert offsets into a valid format for contour forests
  std::vector<ftm::idVertex> voffsets(numberOfVertices);
  std::copy(offsets,offsets+numberOfVertices,voffsets.begin());

  // get contour tree
  ftm::FTMTreePP contourTree;
  contourTree.setupTriangulation(triangulation_, false);
  contourTree.setVertexScalars(inputScalars_);
  contourTree.setTreeType(ftm::TreeType::Join_Split);
  contourTree.setVertexSoSoffsets(voffsets.data());
  contourTree.setThreadNumber(threadNumber_);
  contourTree.setSegmentation(false);
  contourTree.build<scalarType>();

  // get persistence pairs
  std::vector<std::tuple<ftm::idVertex, ftm::idVertex, scalarType>> JTPairs;
  std::vector<std::tuple<ftm::idVertex,ftm::idVertex,scalarType>> STPairs;
  contourTree.computePersistencePairs<scalarType>(JTPairs, true);
  contourTree.computePersistencePairs<scalarType>(STPairs, false);

  // merge pairs
  std::vector<std::tuple<ftm::idVertex, ftm::idVertex, scalarType, bool>> CTPairs(JTPairs.size() +
                                                                        STPairs.size());
  const ftm::idVertex JTSize = JTPairs.size();
  for (ftm::idVertex i = 0; i < JTSize; ++i) {
     const auto& x = JTPairs[i];
     CTPairs[i]    = std::make_tuple(std::get<0>(x), std::get<1>(x), std::get<2>(x), true);
  }
  const ftm::idVertex STSize = STPairs.size();
  for (ftm::idVertex i = 0; i < STSize; ++i) {
     const auto& x       = STPairs[i];
     CTPairs[JTSize + i] = std::make_tuple(std::get<0>(x), std::get<1>(x), std::get<2>(x), false);
  }

  // remove the last pair which is present two times (global extrema pair)
  {
     auto cmp = [](const std::tuple<ftm::idVertex, ftm::idVertex, scalarType, bool>& a,
                   const std::tuple<ftm::idVertex, ftm::idVertex, scalarType, bool>& b) {
        return std::get<2>(a) < std::get<2>(b);
     };

     std::sort(CTPairs.begin(), CTPairs.end(), cmp);
     CTPairs.erase(CTPairs.end() - 1);
  }

  // get the saddle-saddle pairs
  std::vector<std::tuple<SimplexId,SimplexId,scalarType>> pl_saddleSaddlePairs;
  const int dimensionality=triangulation_->getDimensionality();
  if(dimensionality==3 and ComputeSaddleConnectors){
    MorseSmaleComplex3D morseSmaleComplex;
    morseSmaleComplex.setDebugLevel(debugLevel_);
    morseSmaleComplex.setThreadNumber(threadNumber_);
    morseSmaleComplex.setupTriangulation(triangulation_);
    morseSmaleComplex.setInputScalarField(inputScalars_);
    morseSmaleComplex.setInputOffsets(inputOffsets_);
    morseSmaleComplex.computePersistencePairs<scalarType>(JTPairs, STPairs, pl_saddleSaddlePairs);
  }

  // get persistence diagrams
  computeCTPersistenceDiagram<scalarType>(
      contourTree,CTPairs, CTDiagram, scalars);

  // add saddle-saddle pairs to the diagram if needed
  if(dimensionality==3 and ComputeSaddleConnectors){
    for(const auto& i : pl_saddleSaddlePairs){
      const ftm::idVertex v0=std::get<0>(i);
      const ftm::idVertex v1=std::get<1>(i);
      const scalarType persistenceValue=std::get<2>(i);

      std::tuple<ftm::idVertex, ftm::NodeType, ftm::idVertex, ftm::NodeType, scalarType, ftm::idVertex> t;

      std::get<0>(t)=v0;
      std::get<1>(t)=ftm::NodeType::Saddle1;
      std::get<2>(t)=v1;
      std::get<3>(t)=ftm::NodeType::Saddle2;
      std::get<4>(t)=persistenceValue;
      std::get<5>(t)=1;

      CTDiagram.push_back(t);
    }
  }

  // finally sort the diagram
  sortPersistenceDiagram(CTDiagram, scalars, offsets);

  return 0;
}

#endif // PERSISTENCEDIAGRAM_H
