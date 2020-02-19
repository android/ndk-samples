Android Neural Networks API Sample: Basic
======
Android Neural Networks API (NN API) Sample demonstrates basic usages of NN API with a simple model that consists of three operations: two additions and a multiplication.

The sums created by the additions are the inputs to the multiplication. In essence, we are creating a graph that computes: (tensor0 + tensor1) * (tensor2 + tensor3).
```java
tensor0 ---+
           +--- ADD ---> intermediateOutput0 ---+
tensor1 ---+                                    |
                                                +--- MUL---> output
tensor2 ---+                                    |
           +--- ADD ---> intermediateOutput1 ---+
tensor3 ---+
```

Two of the four tensors, tensor0 and tensor2 being added are constants, defined in the model. They represent the weights that would have been learned during a training process, loaded from model_data.bin.

The other two tensors, tensor1 and tensor3 will be inputs to the model. Their values will be provided when we execute the model. These values can change from execution to execution.

Besides the two input tensors, an optional fused activation function can also be defined for ADD and MUL. In this example, we'll simply set it to NONE.

The model then has 8 operands:
- 2 tensors that are inputs to the model. These are fed to the two ADD operations.
- 2 constant tensors that are the other two inputs to the ADD operations.
- 1 fuse activation operand reused for the ADD operations and the MUL operation.
- 2 intermediate tensors, representing outputs of the ADD operations and inputs to the MUL operation.
- 1 model output.

Screenshots
-----------
<img src="screenshot.png" width="480">
