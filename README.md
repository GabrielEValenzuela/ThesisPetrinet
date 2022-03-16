# Petrinet Simulator & Emulator
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](https://www.repostatus.org/badges/latest/active.svg)](https://www.repostatus.org/#active)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![](https://tokei.rs/b1/github/GabrielEValenzuela/ThesisPetrinet)](https://github.com/GabrielEValenzuela/ThesisPetrinet)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/GabrielEValenzuela/ThesisPetrinet.svg)](http://isitmaintained.com/project/GabrielEValenzuela/ThesisPetrinet "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/GabrielEValenzuela/ThesisPetrinet.svg)](http://isitmaintained.com/project/GabrielEValenzuela/ThesisPetrinet "Percentage of issues still open")

## Petrinet definition
A Petri net is a particular kind of directed bipartite graph (or digraph) together
with an initial state called the initial marking. It contains two types of nodes,
**places**, and **transitions**. In graphical representation, places are drawn as circles, and transitions as bars. Arcs are either from a place to a transition or
from a transition to a place. Arcs are labeled with their weights (positive integers).
Labels for unity weight are usually omitted. A marking (state) assigns to each
place a nonnegative integer *k*; we say that **p** is marked with *k* tokens. Pictorially, we place *k* black dots (tokens) in place **p**. If *k* is large, one can simply write the number *k* inside **p** to represent *k* tokens. A marking is denoted by **M**, an *m-vector*, where *m* is the total number of places. The number of tokens in **p** is denoted by **M(p)**.

## Project description
This project implements the [**Generalized state equation for non-autonomous Petri nets with different types of arcs**](https://www.researchgate.net/publication/329130744_Generalized_state_equation_for_non-autonomous_Petri_nets_with_different_types_of_arcs) for a concurrent monitor algorithm and the minimal coberability tree algorithm, using modern C++.

## Structure

``` text.
├── CMakeLists.txt
├── include
│   ├── AgentFactory.hpp
│   ├── Agent.hpp
│   ├── AgentImmediate.hpp
│   ├── AlgorithmEngine.hpp
│   ├── AlgorithmMinCov.hpp
│   ├── Engine.hpp
│   ├── EventGenerator.hpp
│   ├── HourTimeLogic.hpp
│   ├── Logger.hpp
│   ├── MicrTimeLogic.hpp
│   ├── MilliTimeLogic.hpp
│   ├── MinuTimeLogic.hpp
│   ├── Monitor.hpp
│   ├── NanoTimeLogic.hpp
│   ├── NodeState.hpp
│   ├── OmegaTransition.hpp
│   ├── OutputParser.hpp
│   ├── PetriBuilder.hpp
│   ├── PetriDirector.hpp
│   ├── PetriNetwork.hpp
│   ├── PlaceTransitionNet.hpp
│   ├── Queue.hpp
│   ├── ReadFile.hpp
│   ├── ReadJSON.hpp
│   ├── ReadXML.hpp
│   ├── SecoTimeLogic.hpp
│   ├── TemporalHourAgent.hpp
│   ├── TemporalMicroAgent.hpp
│   ├── TemporalMilliAgent.hpp
│   ├── TemporalMinuteAgent.hpp
│   ├── TemporalNanoAgent.hpp
│   ├── TemporalPlaceTransitionNet.hpp
│   ├── TemporalSecondAgent.hpp
│   └── TimeLogic.hpp
├── lib
│   ├── AnalysisEngine.hpp
│   ├── args.hxx
│   ├── entityPN.hpp
│   ├── json.hpp
│   ├── MathEngine.hpp
│   ├── PetriMatrix.hpp
│   ├── pugiconfig.hpp
│   ├── pugixml.cpp
│   ├── pugixml.hpp
│   ├── sha1.hpp
├── LICENSE
├── README.md
├── src
│   ├── AgentFactory.cpp
│   ├── AgentImmediate.cpp
│   ├── AlgorithmMinCov.cpp
│   ├── Engine.cpp
│   ├── main.cpp
│   ├── Monitor.cpp
│   ├── PetriNetwork.cpp
│   ├── PlaceTransitionNet.cpp
│   ├── Queue.cpp
│   ├── ReadJSON.cpp
│   ├── ReadXML.cpp
│   ├── TemporalHourAgent.cpp
│   ├── TemporalMicroAgent.cpp
│   ├── TemporalMilliAgent.cpp
│   ├── TemporalMinuteAgent.cpp
│   ├── TemporalNanoAgent.cpp
│   ├── TemporalPlaceTransitionNet.cpp
│   └── TemporalSecondAgent.cpp
```

## Build and install
``` bash
> mkdir build && cd build
> cmake .. -G[Ninja] -DCMAKE_BUILD_TYPE=[Debug | Coverage | Release]
> make [ninja]
> ./Petrinet -opt [file_input]
```

## Structure of JSON PN and configuration file used for simulation
```json
{
	"network": {
        "id": "name_PN",
        "amount_places": N,
        "amount_transitions": M,
        "time_scale": "none || unit",
        "is_temporal": false || true,
        "network_type": "discrete || continuos || colored"
    },
    "places":[
    {
    	"index":N,
	"type":Type,
	"initial_marking":K
    },...],
    "transitions":[
    {
    	"index":M,
	"type":Type,
	"guard":true || false,
	"event": true || false
    },...],
    "arcs":[
    {
    	"type":Type,
	"from_place": true || false,
	"source": i,
	"target": j,
	"weight": W
    },...]
}

//Configuration
{
	"firesequence_agents": [
      {
        "type": "immediate",
        "sequence": [...]
      },...],
      "max_fire":MAX
}
```

### Contribution and feedback
Please, use discussion to get more information about the project or the algorithms or open a issue if you want.
