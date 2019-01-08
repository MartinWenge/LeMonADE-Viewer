/*--------------------------------------------------------------------------------
 _______________
|   /       \   | L   attice-based  | LeMonADE-Viewer:
|  /   ooo   \  | e   xtensible     | ----------------
| /  o\.|./o  \ | Mon te-Carlo      | An Open Source Visualization Tool for
|/  o\.\|/./o  \| A   lgorithm and  | for BFM-files generated by LeMonADE-software.
|  oo---0---oo  | D   evelopment    | See: https://github.com/LeMonADE-project/LeMonADE
|\  o/./|\.\o  /| E   nvironment    |
| \  o/.|.\o  / | -                 |
|  \   ooo   /  | Viewer            | Copyright (C) 2013-2015 by
|___\_______/___|                   | LeMonADE-Viewer Principal Developers (see AUTHORS)
----------------------------------------------------------------------------------

This file is part of LeMonADE-Viewer.

LeMonADE-Viewer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LeMonADE-Viewer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LeMonADE-Viewer.  If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <sstream>
#include <algorithm>    // std::find_if
#include <cstddef>      // NULL
#include <cstdlib>      // std::strtod
#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <functional>
#include <queue>
#include <vector>
#include <math.h>

#include <LeMonADE/utility/RandomNumberGenerators.h>

template<class IngredientsType, class MoleculesType>
class LineCommandBase
{
public:

	virtual ~LineCommandBase(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& str)=0;

	static std::vector<std::string> tokenize1Parameter(const std::string& str,
			char delim) {
		std::vector<std::string> tokens;
		std::stringstream mySstream(str);
		std::string temp;

		while (getline(mySstream, temp, delim)) {
			tokens.push_back(temp);
		}

		return tokens;

	}

	static std::vector<std::string> tokenize2Parameter(const std::string& str,	char delim1, char delim2) {
		std::vector<std::string> tokens;
		std::stringstream mySstream(str);
		std::string temp;

		while (getline(mySstream, temp, delim1)) {

			if(temp.size() != 0)
			{
				std::stringstream mySstream2(temp);
				std::string temp2;
				while (getline(mySstream2, temp2, delim2)) {
					//printf("%s \n", temp2.c_str());
					if(temp2.size() != 0)
						tokens.push_back(temp2);
				}
			}

		}

		return tokens;

	}

	static std::vector<std::string> tokenize3Parameter(const std::string& str,
			char delim1, char delim2, char delim3) {
		std::vector<std::string> tokens;
		std::stringstream mySstream(str);
		std::string temp;

		while (getline(mySstream, temp, delim1)) {
			std::stringstream mySstream2(temp);
			std::string temp2;
			while (getline(mySstream2, temp2, delim2)) {
				std::stringstream mySstream3(temp2);
				std::string temp3;
				while (getline(mySstream3, temp3, delim3)) {

					//printf("%s \n", temp2.c_str());
					tokens.push_back(temp3);
				}
			}

		}

		return tokens;

	}
};

int readColor
(
    std::string const & sr,
    float     * const   r ,
    float     * const   g ,
    float     * const   b ,
    std::string const & sCallingCommand = ""
)
{
    /* @todo tokenize3Parameter could be a standalone function, the template parameters are not even needed ...! */
    std::vector< std::string >  sColors = LineCommandBase<int,int>::tokenize3Parameter( sr, '(', ',' , ')' );

    unsigned int const nChannels = 3;
    if ( sColors.size() != nChannels )
    {
        std::stringstream msg;
        msg << "Error ";
        if ( ! sCallingCommand.empty() )
            msg << "in command '" << sCallingCommand << "' ";
        msg << "when reading the color in rgb(x,x,x) format. ";
        msg << "Expected exactly " << nChannels << " colors, but got " << sColors.size() << "!";
        std::cerr << msg.str() << "\n";
        return 1;
    }

    /* convert strings to floats (failed colors are set to 0 by strtod) */
    char * eos = NULL;
    unsigned int failedReads = 0;
    float * c[] = { r,g,b };
    for ( unsigned int i = 0; i < nChannels; ++i )
    {
        *c[i] = strtod( sColors[i].c_str(), &eos );
        failedReads |= ( eos == sColors[i] ) << i;
    }
    if ( failedReads )
    {
        std::stringstream msg;
        msg << "Error ";
        if ( ! sCallingCommand.empty() )
            msg << "in command '" << sCallingCommand << "' ";
        msg << "when reading the color: ";
        if ( failedReads & ( 1 << 0 ) ) msg << "red"  ;
        if ( failedReads & ( 1 << 1 ) ) msg << "green";
        if ( failedReads & ( 1 << 2 ) ) msg << "blue" ;
        msg << "!";
        std::cerr << msg.str() << "\n";
        return 2;
    }

    /* check for correct ranges */
    if ( *r > 1 || *g > 1 || *b > 1 )
    {
        *r /= 255;
        *g /= 255;
        *b /= 255;
    }
    if ( ! ( 0 <= *r && *r <= 1 && 0 <= *g && *g <= 1 && 0 <= *b && *b <= 1 ) )
    {
        std::stringstream msg;
        msg << "Error ";
        if ( ! sCallingCommand.empty() )
            msg << "in command '" << sCallingCommand << "' ";
        msg << "when reading the color in rgb(x,x,x) format. "
            << "Colors must be either in range [0,1] or [0,255]. "
            << "The latter gets automatically downscaled to [0,1]. "
            << "But the input received is: (" << *r << "," << *g << "," << *b << ")";
        std::cerr << msg.str() << "\n";
        return 3;
    }

    return 0;
}

// handles
// !setColor:idx1-idx2=(r,g,b)
// !setColor:all=(r,g,b)
// !setColor:BG=(r,g,b)

template<class IngredientsType, class MoleculesType>
class CommandSetColor : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColor(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColor");

		//get monomer information
		std::vector<std::string> mono = LineCommandBase<IngredientsType, MoleculesType>::tokenize1Parameter(vex[1], '-');

		//mono index has 1 or 2 tokens
		if((mono.size() ==0) || (mono.size() > 2) )
			return std::string("error in command !setColor:monomerindex");

        float red, green, blue;
        readColor( vex[2], &red, &green, &blue, "!setColor" );

		//color all monomers
		if(mono.size()==1 && (!mono[0].compare("all")))
		{
			//std::cout << "color all monomers"<< std::endl;

			for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
				ingredients.modifyMolecules()[i].setColor( red ,green , blue);

			return std::string("apply color to all monomers");
		}

		//color the background
		if(mono.size()==1 && (!mono[0].compare("BG")))
		{
			//std::cout << "color BG"<< std::endl;
			//set background color
			ingredients.setBGcolor(red, green, blue);

			return std::string("apply color to BG");
		}

		// label monomer in range
		if(mono.size()==2)
		{

			uint32_t mono1 = 0;
			uint32_t mono2 = 0;

			std::istringstream ss1(mono[0]);
			ss1 >> mono1;

			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor:1.st monomer index");
			}

			if(mono1 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 1.st monomer index");
			}

			std::istringstream ss2(mono[1]);
			ss2 >> mono2;

			if(mono2 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 2.nd monomer index");
			}
			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor:2.st monomer index");
			}

			if(mono1 < mono2)
			{
				for(uint32_t i = mono1; i <= mono2; i++)
					ingredients.modifyMolecules()[i].setColor( red ,green , blue);
			}
			else
			{for(uint32_t i = mono2; i <= mono1; i++)
				ingredients.modifyMolecules()[i].setColor( red ,green , blue);
			}

			return std::string("apply color monomers");
		}
		return std::string("could not color anything");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetColorAttributes : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorAttributes(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorAttributes " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorAttributes");

		int32_t attribute=0;

		std::istringstream ccAtt(vex[1]);
		ccAtt >> attribute;

		//attribute is wrong
		if(ccAtt.fail()){
			return std::string("error in command !setColorAttributes: attribute");
		}

        float red, green, blue;
        readColor( vex[2], &red, &green, &blue, "!setColorAttributes" );

		//color all monomers
		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules()[i].getAttributeTag() == attribute)
				ingredients.modifyMolecules()[i].setColor( red ,green , blue);

		return std::string("apply color to all monomers with attribute");
	}
};


template<class IngredientsType, class MoleculesType>
class CommandSetColorLinks : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorLinks(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorLinks " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorLinks");

		int32_t numLinks=0;

		std::istringstream ccLinks(vex[1]);
		ccLinks >> numLinks;

		//numLinks is wrong
		if(ccLinks.fail()){
			return std::string("error in command !setColorLinks: numLinks");
		}

        float red, green, blue;
        readColor( vex[2], &red, &green, &blue, "!setColorLinks" );

		//color all monomers


		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
		{
			if(ingredients.modifyMolecules().getNumLinks(i) == numLinks)
				ingredients.modifyMolecules()[i].setColor( red ,green , blue);
		}

		return std::string("apply color to all monomers with numLinks");
	}
};




template<class IngredientsType, class MoleculesType>
class CommandSetColorTopology : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorTopology(){};

    typedef uint32_t NodeIdx;
    typedef std::map<NodeIdx, int> Nodelist;
    typedef std::map<NodeIdx, Nodelist> Graph;
    //typedef std::pair<NodeIdx, Nodelist> Node;
    typedef std::pair<int, NodeIdx> Edge; //! (tentative distance, idxNode)
    typedef std::vector<NodeIdx> NodeVector;

    void dijkstra(Graph &graph, NodeIdx source, Nodelist &distance);

    void findCentersOfTree(Graph &graph,  Nodelist degreeNodes, NodeVector &centerNodes);

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorLinks " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorTopology");

        float redStart, greenStart, blueStart;
        readColor( vex[1], &redStart, &greenStart, &blueStart, "!setColorTopology (start)" );
        float redEnd, greenEnd, blueEnd;
        readColor( vex[2], &redEnd, &greenEnd, &blueEnd, "!setColorTopology (end)" );



		/*  ################################################################  */

		// if the coloring should be calculated using the systems larges topological distance, replace MonomerGroupsVector molecules_type  by ingredients and the true index by the monomer index
		// do the coloring groupwise
		for(uint32_t groupIdx=0; groupIdx<molecules_type.size(); groupIdx++ ){

			//typedef std::map<NodeIdx, Nodelist> Graph;
			Graph treeGraph;
			//typedef std::map<NodeIdx, int> Nodelist;
			Nodelist  degreeNode; // Degree of node
			for( uint32_t k=0; k<molecules_type[groupIdx].size(); k++){
				// get the index from the group
				int32_t monoIdx(molecules_type[groupIdx].trueIndex(k));
				for( uint32_t link=0; link< ingredients.getMolecules().getNumLinks(monoIdx); link++){
					// connect from idxAA to idxBB with value ZZ
					// all (symmetric) connections
					treeGraph[monoIdx][ingredients.getMolecules().getNeighborIdx(monoIdx, link)]=1;
				}
				// fill with degree of nodes
				degreeNode[monoIdx]=ingredients.getMolecules().getNumLinks(monoIdx);
			}

			//finding the center of the tree (either one or two)
			//typedef std::vector<NodeIdx> NodeVector;
			NodeVector centerNodes;
			findCentersOfTree(treeGraph, degreeNode, centerNodes);

			//calculate the topological distance of center to all nodes and fill histogram
			NodeVector::iterator itv;

			for(itv=centerNodes.begin(); itv!=centerNodes.end(); ++itv){
				NodeIdx startNode = (*itv);


            	// idx -> distance from center
	        	Nodelist topo_distance;
	        	dijkstra(treeGraph, startNode, topo_distance);
	
            	int maxTopologicalDistance = 0;


	       		Nodelist::iterator it;
	       		for(it=topo_distance.begin(); it!=topo_distance.end(); ++it){
            	    //find the maximum topological distance from the tree center
            	    if(maxTopologicalDistance < it->second)
            	        maxTopologicalDistance=it->second;
            	}


	        std::cout << "startNode -> node => distance" <<std::endl;


              std::cout<< startNode << " -> max topological distance " << maxTopologicalDistance <<std::endl;

              //color the structure
              for(it=topo_distance.begin(); it!=topo_distance.end(); ++it){

	       		//  std::cout<< startNode << " -> " << it->first << "  => "<<it->second<<std::endl;

                  // red part
                  double redFraction = redStart + (redEnd-redStart)*it->second/(1.0*maxTopologicalDistance);

                  // green part
                  double greenFraction = greenStart + (greenEnd-greenStart)*it->second/(1.0*maxTopologicalDistance);

                  // blue part
                  double blueFraction = blueStart + (blueEnd-blueStart)*it->second/(1.0*maxTopologicalDistance);


                  ingredients.modifyMolecules()[it->first].setColor( redFraction ,greenFraction , blueFraction);
              }




	      	} // end loop Node vector
		} // end loop monomerGroups


		return std::string("apply color to all monomers with numTopology");
	}
};

template<class IngredientsType, class MoleculesType>
void CommandSetColorTopology<IngredientsType, MoleculesType>::findCentersOfTree(Graph &graph, Nodelist degreeNodes, NodeVector &centerNodes){

	std::queue <NodeIdx> queueLeaves; // Queue for algorithm holding (reduced) leaves
	std::queue <NodeIdx> queueNextLeaves; // Queue for algorithm holding next (reduced) leaves
	std::queue <NodeIdx> tmpOldLeaves; // Queue for algorithm holding (reduced) leaves

	// Fill and start from leaves

	Nodelist::iterator it;
	for(it=degreeNodes.begin(); it!=degreeNodes.end(); ++it)
	{
	        int degreeOfNode = it->second;
	        NodeIdx labelNode = it->first;

	        if (degreeOfNode == 1)
	        {
	        	queueLeaves.push(labelNode);
	        }
	}

	NodeIdx Seperator = -1;

	queueLeaves.push(Seperator);

	//  while ( (queueLeaves.front()!=Seperator)||(queueLeaves.size() > 2) ) {
	while ( (queueLeaves.size() > 0) ) {
		  NodeIdx node = queueLeaves.front();
		  queueLeaves.pop();

		  if( node == Seperator)
		  {
			  if(queueNextLeaves.empty())
				  break;

			 // std::cout << "next iteration: " << node << std::endl;
			  tmpOldLeaves = queueNextLeaves;
			  queueLeaves = queueNextLeaves;

			  if(queueNextLeaves.size() >0)
				  queueLeaves.push(Seperator);

			  queueNextLeaves=std::queue <NodeIdx>();

			  continue;
		  }


	 	      int dN_old = degreeNodes[node];

	 	      degreeNodes[node]--;

	 	     // new subgraph of all neighbors
	 	     Nodelist tempgraph=graph[node];

	 	     // connected nodes

	 	    // std::cout << "connected nodes from: " << node << std::endl;

	 	     Nodelist::iterator it;
	 	     for(it=tempgraph.begin(); it!=tempgraph.end(); ++it){

	 	    //	std::cout << it->first << " with degree: " << degreeNodes[it->first];

	 	    	 if(degreeNodes[it->first] > 0)
	 	    	 {
	 	    		degreeNodes[it->first]--;

	 	    	//	std::cout << " reduced to degree: " << degreeNodes[it->first];
	 	    	 }

	 	    	 if(degreeNodes[it->first] == 1)
	 	    	 {
	 	    		//queueLeaves.push(it->first);
	 	    		queueNextLeaves.push(it->first);

	 	    	//	std::cout << " added to queue the idx: " << (it->first);
	 	    	 }

	 	    	//std::cout << std::endl;
	 	     }


	 	  }

	std::cout << "center queue contains: ";
		  while (!tmpOldLeaves.empty())
		  {
			  centerNodes.push_back(tmpOldLeaves.front());
			  std::cout << ' ' <<  tmpOldLeaves.front();
			  tmpOldLeaves.pop();
		  }
		  std::cout << '\n';
}

template<class IngredientsType, class MoleculesType>
void CommandSetColorTopology<IngredientsType, MoleculesType>::dijkstra(Graph &graph, NodeIdx source, Nodelist &distance){

	distance.clear(); //! clear all tentative distance information

	//! list of all nodes to be visit and addressed already with least distance on top
  std::priority_queue<Edge, std::vector<Edge>, std::greater<Edge> > queueNode;

  // starting node with tentative distance=0, starting node = source
  queueNode.push( Edge(0, source) );

  while(!queueNode.empty()){

	  //get the element with least tentative distance
    Edge tmped=queueNode.top();

    // access the node index
    NodeIdx tmpnl=tmped.second;

    // removes the top element
    queueNode.pop();

    // if the node never visited before
    if(distance.count(tmpnl)==0){

    	// tentative distance to the recent node
      int dist=tmped.first;

      // set the tentative distance to the node
      distance[tmpnl]=dist;

      // new subgraph of all neighbors
      Nodelist tempgraph=graph[tmpnl];

      Nodelist::iterator it;
      for(it=tempgraph.begin(); it!=tempgraph.end(); ++it){
        int distint=it->second;
        NodeIdx distlabel=it->first;
        queueNode.push(Edge(dist+distint, distlabel));
      }

    }
  }

}

template<class IngredientsType, class MoleculesType>
class CommandSetVisible : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetVisible(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetVisible in CommandSetVisible " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setVisible");

		//get monomer information
		std::vector<std::string> mono = LineCommandBase<IngredientsType, MoleculesType>::tokenize1Parameter(vex[1], '-');

		//mono index has 1 or 2 tokens
		if((mono.size() ==0) || (mono.size() > 2) )
			return std::string("error in command !setVisible: monomer index");

		//get visibility information
		int visible =0;

		std::istringstream ccVisible(vex[2]);
		ccVisible >> visible;

		//first mono index is wrong
		if(ccVisible.fail()){
			return std::string("error in command !setVisible: visibility");
		}

		//color all monomers
		if(mono.size()==1 && (!mono[0].compare("all")))
		{
			std::cout << "set visible all monomers"<< std::endl;

			for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
				ingredients.modifyMolecules()[i].setVisible(bool(visible));

			return std::string("set visibility to all monomers");
		}

		// label monomer in range
		if(mono.size()==2)
		{
			uint32_t mono1 = 0;
			uint32_t mono2 = 0;

			std::istringstream ss1(mono[0]);
			ss1 >> mono1;

			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor:1.st monomer index");
			}

			if(mono1 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 1.st monomer index");
			}

			std::istringstream ss2(mono[1]);
			ss2 >> mono2;

			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor: 2.st monomer index");
			}

			if(mono2 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 2.nd monomer index");
			}

			if(mono1 < mono2)
			{
				for(uint32_t i = mono1; i <= mono2; i++)
					ingredients.modifyMolecules()[i].setVisible(bool(visible));
			}
			else
			{for(uint32_t i = mono2; i <= mono1; i++)
				ingredients.modifyMolecules()[i].setVisible(bool(visible));
			}

			return std::string("apply visibility to monomers");
		}
		return std::string("could not set visibility to monomers");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetVisibleAttributes : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetVisibleAttributes(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetVisible in CommandSetVisibleAttributes " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setVisibleAttributes");

		int32_t attribute=0;

		std::istringstream ccAtt(vex[1]);
		ccAtt >> attribute;

		//attribute is wrong
		if(ccAtt.fail()){
			return std::string("error in command !setColorAttributes: attribute");
		}

		//get visibility information
		int visible =0;

		std::istringstream ccVisible(vex[2]);
		ccVisible >> visible;

		if(ccVisible.fail()){
			return std::string("error in command !setVisibleAttributes: Visibility");
		}



		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules()[i].getAttributeTag() == attribute)
				ingredients.modifyMolecules()[i].setVisible( (bool(visible)));

		return std::string("apply visiblility to all monomers with attribute");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetVisibleLinks : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetVisibleLinks(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetVisible in CommandSetVisibleLinks " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setVisibleLinks");

		int32_t numLinks=0;

		std::istringstream ccLinks(vex[1]);
		ccLinks >> numLinks;

		//attribute is wrong
		if(ccLinks.fail()){
			return std::string("error in command !setColorLinks: numLinks");
		}

		//get visibility information
		int visible =0;

		std::istringstream ccVisible(vex[2]);
		ccVisible >> visible;

		if(ccVisible.fail()){
			return std::string("error in command !setVisibleLinks: Visibility");
		}

		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules().getNumLinks(i) == numLinks)
				ingredients.modifyMolecules()[i].setVisible( (bool(visible)));

		return std::string("apply visiblility to all monomers with attribute");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetColorVisibility : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorVisibility(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorVisibility " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorVisibility");

		int32_t Visibility=0;

		std::istringstream ccVis(vex[1]);
		ccVis >> Visibility;

		//attribute is wrong
		if(ccVis.fail()){
			return std::string("error in command !setColorVisibility: Visibility");
		}
        float red, green, blue;
        readColor( vex[2], &red, &green, &blue, "!setColorVisibility" );


		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules()[i].isVisible() == (bool(Visibility)))
				ingredients.modifyMolecules()[i].setColor( red ,green , blue);

		return std::string("apply color to all (in)visible monomers ");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetVisibleGroups : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetVisibleGroups(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetVisible in CommandSetVisibleGroups " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 4 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setVisibleGroups");

		//get monomer information
		std::vector<std::string> Groups = LineCommandBase<IngredientsType, MoleculesType>::tokenize1Parameter(vex[1], '-');

		//mono index has 1 or 2 tokens
		if((Groups.size() != 2) )
			return std::string("error in command !setVisible: monomer index");

		uint32_t group1 = 0;
		uint32_t group2 = 0;

		std::istringstream ss1(Groups[0]);
		ss1 >> group1;

		//first mono index is wrong
		if(ss1.fail()){
			return std::string("error in command !setColor:1.st group index");
		}

		if((group1 < 0) || (group1 >= molecules_type.size()))
		{
			return std::string("error in command !setColor: out-of-range 1.st group index");
		}

		std::istringstream ss2(Groups[1]);
		ss2 >> group2;

		//first mono index is wrong
		if(ss1.fail()){
			return std::string("error in command !setColor: 2.st group index");
		}

		if((group2 < 0) || (group2 >= molecules_type.size()))
		{
			return std::string("error in command !setColor: out-of-range 2.nd monomer index");
		}

		//get visibility information
		int visible =0;

		std::istringstream ccGroup(vex[2]);
		ccGroup >> visible;

		if(ccGroup.fail()){
			return std::string("error in command !setVisibleGroups: Visibility");
		}



		if(group1 < group2)
		{
			for(uint32_t i = group1; i <= group2; i++)
				for(size_t m=0; m< molecules_type[i].size(); ++m)
					ingredients.modifyMolecules()[molecules_type[i].trueIndex(m)].setVisible( (bool(visible)));


		}
		else
		{
			for(uint32_t i = group2; i <= group1; i++)
				for(size_t m=0; m< molecules_type[i].size(); ++m)
					ingredients.modifyMolecules()[molecules_type[i].trueIndex(m)].setVisible( (bool(visible)));

		}

		return std::string("visible color to all monomers within Groups");

	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetColorGroups : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorGroups(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorGroups " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorGroups");

		int32_t group=0;

		std::istringstream ccGroup(vex[1]);
		ccGroup >> group;

		//Group is wrong
		if(ccGroup.fail()){
			return std::string("error in command !setColorGroups: Groups");
		}

		if((group < 0) || (group >= molecules_type.size()))
		{
			return std::string("error in command !setColorGroups: Groups exceed range");
		}
        float red, green, blue;
        readColor( vex[2], &red, &green, &blue, "!setColorGroups" );
		for(size_t m=0; m< molecules_type[group].size(); ++m){
			ingredients.modifyMolecules()[molecules_type[group].trueIndex(m)].setColor( red ,green , blue);

		}

		return std::string("apply color to all monomers within Groups");

	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetColorGroupsRandom : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetColorGroupsRandom(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetColorGroupsRandom " << std::endl;

		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 1 tokens
		if(vex.size() !=1 )
			return std::string("error in command !setColorGroups");

		RandomNumberGenerators RNG;

		float red = 0.0f;
		float green = 0.0f;
		float blue = 0.0f;

		//std::cout << "color all monomers whithin group"<< std::endl;
		for(size_t n=0; n< molecules_type.size(); ++n){

			red = RNG.r250_drand();
			green = RNG.r250_drand();
			blue = RNG.r250_drand();

			for(size_t m=0; m< molecules_type[n].size(); ++m){
				ingredients.modifyMolecules()[molecules_type[n].trueIndex(m)].setColor( red ,green , blue);

			}
		}
		return std::string("apply color to all monomers within Groups");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetRadius : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetRadius(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		std::cout << "!SetRadius in CommandSetRadius " << std::endl;

		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setRadius");

		//get monomer information
		std::vector<std::string> mono = LineCommandBase<IngredientsType, MoleculesType>::tokenize1Parameter(vex[1], '-');

		//mono index has 1 or 2 tokens
		if((mono.size() ==0) || (mono.size() > 2) )
			return std::string("error in command !setRadius:monomerindex");

		//get radius information
		float radius = 0.0f;

		std::istringstream ccRadius(vex[2]);
		ccRadius >> radius;

		//first mono index is wrong
		if(ccRadius.fail()){
			return std::string("error in command !setRadiusr:Radius");
		}

		if(radius <= 0.0f)
		{
			return std::string("error: radius <= 0.0f");
		}

		//color all monomers
		if(mono.size()==1 && (!mono[0].compare("all")))
		{
			std::cout << "color all monomers"<< std::endl;

			for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
				ingredients.modifyMolecules()[i].setRadius( radius);

			return std::string("apply radius to all monomers");
		}

		// label monomer in range
		if(mono.size()==2)
		{
			//uint32_t mono1 = atoi(mono[0].c_str());
			//uint32_t mono2 = atoi(mono[1].c_str());

			uint32_t mono1 = 0;
			uint32_t mono2 = 0;

			std::istringstream ss1(mono[0]);
			ss1 >> mono1;

			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor:1.st monomer index");
			}

			if(mono1 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 1.st monomer index");
			}

			std::istringstream ss2(mono[1]);
			ss2 >> mono2;

			if(mono2 >= ingredients.modifyMolecules().size())
			{
				return std::string("error in command !setColor: out-of-range 2.nd monomer index");
			}
			//first mono index is wrong
			if(ss1.fail()){
				return std::string("error in command !setColor:2.st monomer index");
			}


			if(mono1 < mono2)
			{
				for(uint32_t i = mono1; i <= mono2; i++)
					ingredients.modifyMolecules()[i].setRadius( radius);
			}
			else
			{for(uint32_t i = mono2; i <= mono1; i++)
				ingredients.modifyMolecules()[i].setRadius( radius);
			}

			return std::string("apply radius monomers");
		}
		return std::string("could not change radius monomers");
	}
};


template<class IngredientsType, class MoleculesType>
class CommandSetRadiusAttributes : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetRadiusAttributes(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetRadius in CommandSetRadiusAttributes " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setColorAttributes");

		int32_t attribute=0;

		std::istringstream ccAtt(vex[1]);
		ccAtt >> attribute;

		//attribute is wrong
		if(ccAtt.fail()){
			return std::string("error in command !setColorAttributes: attribute");
		}

		//get radius information
		float radius = 0.0f;

		std::istringstream ccRadius(vex[2]);
		ccRadius >> radius;

		//first mono index is wrong
		if(ccRadius.fail()){
			return std::string("error in command !setRadius:Radius");
		}

		if(radius <= 0.0f)
		{
			return std::string("error: radius <= 0.0f");
		}

		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules()[i].getAttributeTag() == attribute)
				ingredients.modifyMolecules()[i].setRadius( radius);

		return std::string("apply radius to all monomers with attribute");

	}
};

template<class IngredientsType, class MoleculesType>
class CommandSetRadiusLinks : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetRadiusLinks(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetRadiusLinks " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setRadiusLinks");

		int32_t numLinks=0;

		std::istringstream ccLinks(vex[1]);
		ccLinks >> numLinks;

		//numLinks is wrong
		if(ccLinks.fail()){
			return std::string("error in command !setRadiusLinks: numLinks");
		}

		//get radius information
		float radius = 0.0f;

		std::istringstream ccRadius(vex[2]);
		ccRadius >> radius;

		//first mono index is wrong
		if(ccRadius.fail()){
			return std::string("error in command !setRadius:Radius");
		}

		if(radius <= 0.0f)
		{
			return std::string("error: radius <= 0.0f");
		}


		for(uint32_t i = 0; i < ingredients.modifyMolecules().size(); i++)
			if(ingredients.modifyMolecules().getNumLinks(i) == numLinks)
				ingredients.modifyMolecules()[i].setRadius( radius);

		return std::string("apply radius to all monomers with numLinks");
	}
};


template<class IngredientsType, class MoleculesType>
class CommandSetRadiusGroups : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandSetRadiusGroups(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		//std::cout << "!SetColor in CommandSetRadiusGroups " << std::endl;
		std::vector<std::string> vex = LineCommandBase<IngredientsType, MoleculesType>::tokenize2Parameter(_commandLine, ':', '=');

		//command has 3 tokens
		if(vex.size() !=3 )
			return std::string("error in command !setRadiusGroups");

		int32_t group=0;

		//color all monomers

		std::istringstream ccGroup(vex[1]);
		ccGroup >> group;

		//Group is wrong
		if(ccGroup.fail()){
			return std::string("error in command !setRadiusGroups: Groups");
		}

		if((group < 0) || (group >= molecules_type.size()))
		{
			return std::string("error in command !setColorGroups: Groups exceed range");
		}

		//get radius information
		float radius = 0.0f;

		std::istringstream ccRadius(vex[2]);
		ccRadius >> radius;

		//first mono index is wrong
		if(ccRadius.fail()){
			return std::string("error in command !setRadius:Radius");
		}

		if(radius <= 0.0f)
		{
			return std::string("error: radius <= 0.0f");
		}


		for(size_t m=0; m< molecules_type[group].size(); ++m){
			ingredients.modifyMolecules()[molecules_type[group].trueIndex(m)].setRadius( radius);

		}

		return std::string("apply color to all monomers within Groups");
	}
};

template<class IngredientsType, class MoleculesType>
class CommandGetHelp : public LineCommandBase<IngredientsType, MoleculesType>
{
public:
	virtual ~CommandGetHelp(){};

	virtual std::string executeLineCommand(IngredientsType& ingredients, MoleculesType& molecules_type, std::string& _commandLine)
	{
		std::cout << "!help with known commands" << std::endl;

		return std::string("known commands:\n"
				"!setColor:idx1-idx2=(r,g,b)\n"
				"!setColor:all=(r,g,b)\n"
                "!setColor:BG=(r,g,b)\n"
				"!setColorAttributes:att=(r,g,b)\n"
				"!setColorLinks:numLinks=(r,g,b)\n"
				"!setColorVisibility:vis=(r,g,b)\n"
				"!setColorGroups:idxGroup=(r,g,b)\n"
				"!setColorGroupsRandom\n"
                "!setColorTopology:(rS,gS,bS)=(rE,gE,bE)\n"
				"!setVisible:idx1-idx2=vis\n"
				"!setVisible:all=vis\n"
				"!setVisibleAttributes:att=vis\n"
				"!setVisibleLinks:numLinks=vis\n"
				"!setVisibleGroups:idxG1-idxG2=vis\n"
				"!setRadius:idx1-idx2=radius\n"
				"!setRadius:all=radius\n"
				"!setRadiusAttributes:att=radius\n"
				"!setRadiusLinks:numLinks=radius\n"
				"!setRadiusGroups:idxGroup=radius\n"
				"!exit\n");
	}
};
