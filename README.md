# ScottyPrime

Welcome to ScottyPrime! This 3D graphics software package includes components for interactive mesh editing, realistic path tracing, and dynamic animation. It is built on a code skeleton provided by [cmu&#39;s computer graphics course](http://15462.courses.cs.cmu.edu/fall2021/).

You can visit cmu's [documentation website here](https://cmu-graphics.github.io/Scotty3D/).

## Mesh Editor

This part of the applicaton provides the user with wide toolkit of modeling operations. What I implemented was mostly on the halfedge data structure pointers reassignments (connectivity) and calculations for vertex positions (geometry).

### Local operations

below are some examples of the implemented local operations.

| Face<br />Operations |            Illustration            |            GIF from App            |
| :------------------: | :--------------------------------: | :--------------------------------: |
|       Collapse       | ![](docs/assets/1646616712209.png) | ![](docs/assets/1646615804073.png) |
|        Inset         | ![](docs/assets/1646616362225.png) | ![](docs/assets/1646616258814.png) |
|       Extrude        | ![](docs/assets/1646616780603.png) | ![](docs/assets/1646616245251.png) |
|        Bevel         | ![](docs/assets/1646616798362.png) | ![](docs/assets/1646616204864.png) |
|  Insert<br />Vertex  | ![](docs/assets/1646616668844.png) | ![](docs/assets/1646615787213.png) |

| Edge<br />Operations |            Illustration            |                    GIF from App                     |
| :------------------: | :--------------------------------: | :-------------------------------------------------: |
|       Collapse       | ![](docs/assets/1646617037475.png) | ![1646662692294.png](docs/assets/1646662692294.png) |
|        Erase         | ![](docs/assets/1646617055347.png) | ![1646662687866.png](docs/assets/1646662687866.png) |
|        Split         | ![](docs/assets/1646617009936.png) | ![1646662664993.png](docs/assets/1646662664993.png) |
|         Flip         | ![](docs/assets/1646617003685.png) | ![1646662676129.png](docs/assets/1646662676129.png) |
|        Bevel         | ![](docs/assets/1646617079322.png) |                Yet to be Implemented                |

| Vertex<br />Operations |             Illustration              |                    GIF from App                     |
| :--------------------: | :-----------------------------------: | :-------------------------------------------------: |
|         Erase          | ![img](docs/assets/1646624580973.png) | ![1646661857982.png](docs/assets/1646661857982.png) |
|         Bevel          |  ![](docs/assets/1646624607677.png)   | ![1646661687204.png](docs/assets/1646661687204.png) |
|        Extrude         |  ![](docs/assets/1646624544469.png)   | ![1646661852233.png](docs/assets/1646661852233.png) |
