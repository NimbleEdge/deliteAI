class Tensor:
    def shape(self)->list[int]:
        """
        Returns the shape of the tensor.

        Returns
        ----------
        shape : list[int]
        """
        pass

    def reshape(self,newShape:list[int], Tensor):
        """
        Reshape the tensor to the provided newShape.
        Is is required for the newShape to be compatible with the shape of the original tensor i.e. the total number of elements in both the shapes shoould be same.

        Parameters
        ----------
        newShape : list[int] |  Tensor
            The new shape in which the existing tensor needs to be reshaped.

        Returns
        ----------
        reshapedTensor : Tensor
            Returns the same tensor with the new shape.
        """
        return self

    def sort(self,order:str):
        """
        Sort the tensor based on order. For sort to work the tensor should be 1 dimensional.

        Parameters
        ----------
        order : str
            Order of sorting. Allowed values are "asc" and "desc"

        Returns
        ----------
        sortedTensor : Tensor
            The same tensor in the sorted order.
        """
        return self

    def topk(self, num:int, order:str):
        """
        Return the indices of topk elements of the tensor are sorting based on order. For topk to work the tensor should be 1 dimensional.
        Does not modify the existing tensor.

        Parameters
        ----------
        num : int
            Number of indices to be returned.
        order : str
            Order of sorting. Allowed values are "asc" and "desc"

        Returns
        ----------
        topkIndices : Tensor
            Indices of the topk elements of the tensor, based on sorting order.
        """
        return self

    def argsort(self, order:str):
        """
        List of indices of the sorted tensor based on the order specified. For argsort to work, the tensor should be 1 dimensional.
        Does not modify the existing tensor.

        Parameters
        ----------
        order : str
            Sorting order of tensor. Allowed values are "asc" and "desc"

        Returns
        ----------
        indices : Tensor
            List of indices of the tensor, based on sorting order.
        """
        pass

    def arrange(self, indices:list[int], Tensor):
        """
        Arranges the tensor based on the indices provided as the parameter. The number of elements in the indices list should be less than or equal to the number of elements in the tensor.
        Does not modify existing tensor.
        For arrange to work, the tensor should be 1 dimensional.

        Parameters
        ----------
        indices : list[int] | Tensor
            Indices in which the tensor should be arranged. Allowed types are either a list of ints or a tensor.

        Returns
        ----------
        arrangedTensor : Tensor
            Returns the tensor with values arranged based on indices.
        """
        return self
{{ extract_delitepy_doc_blocks("nimblenet/data_variable/include/nimble_net_data_variable.hpp") }}
{{ extract_delitepy_doc_blocks("nimblenet/data_variable/include/list_data_variable.hpp") }}
