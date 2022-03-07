# ScottyPrime

Welcome to ScottyPrime! This 3D graphics software package includes components for interactive mesh editing, realistic path tracing, and dynamic animation. It is built on a code skeleton provided by [cmu&#39;s computer graphics course](http://15462.courses.cs.cmu.edu/fall2021/).

You can visit cmu's [documentation websithere](https://cmu-graphics.github.io/Scotty3D/).

## Results

### Mesh Editing

This part of the applicaton provides the user with wide toolkit of modeling operations. What I implemented was mostly on the halfedge data structure pointers reassignments (connectivity) and calculations for vertex positions (geometry).

#### Local operations

below are some examples of the implemented local operations.

| Face<br />Operations |             Illustration             |                                                                 GIF from App                                                                 |
| :------------------: | :----------------------------------: | :------------------------------------------------------------------------------------------------------------------------------------------: |
|       Collapse       | ![img](docs/assets/1646616712209.png) |                                              ![1646615804073.png](docs/assets/1646615804073.png)                                              |
|        Inset        | ![img](docs/assets/1646616362225.png) |                                              ![1646616258814.png](docs/assets/1646616258814.png)                                              |
|       Extrude       |  ![](docs/assets/1646616780603.png)  |                                              ![1646616245251.png](docs/assets/1646616245251.png)                                              |
|        Bevel        |  ![](docs/assets/1646616798362.png)  | ![1646616204864.png](docs/assets/1646616204864.png) |
|  Insert<br />Vertex  | ![img](docs/assets/1646616668844.png) | ![1646615787213.png](docs/assets/1646615787213.png) |

| Edge<br />Operations |             Illustration             |     GIF from App     |
| :------------------: | :----------------------------------: | :-------------------: |
|       Collapse       | ![img](docs/assets/1646617037475.png) |                      |
|        Erase        | ![img](docs/assets/1646617055347.png) |                      |
|        Split        | ![img](docs/assets/1646617009936.png) |                      |
|         Flip         | ![img](docs/assets/1646617003685.png) |                      |
|        Bevel        | ![img](docs/assets/1646617079322.png) | Yet to be Implemented |
