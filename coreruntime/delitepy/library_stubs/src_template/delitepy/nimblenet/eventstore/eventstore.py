from delitepy.nimblenet.tensor.tensor import Tensor
from types import FunctionType

class RawEventStore:
    def __init__(self, eventType: str, eventExpiryType: str, eventExpiry: int)->'RawEventStore':
            """
            An object of RawEventStore class is used to define how the events of a particular type should be handled by the sdk. There are two ways it affects the behaviour of the events:
            1. The constructor defines how the events should expire and when.
            2. It can be used in the add_event hook to modify the event coming from frontend, like filtering them out on some condition or adding new fields in the event.

            Parameters
            ----------
            eventType : str
                EventType used by frontend when adding a new event.
            eventExpiryType: str
                How the old events expire i.e. by time or count. It accepts two values "time" and "count".
            eventExpiry: str
                If eventExpiryType is "time" it defines the time in minutes after which events should be deleted. If eventExpiryType is "count" then it denotes the number of latest events which should be kept and rest all should be deleted.
            """
            pass

class EventProcessor:
    """
    An object of EventProcessor class can be used to interact with the on-device event store in which the events are being populated by frontend interactions of the user.Using the EventProcessor object requires 2 steps.
    1. Creating an object of EventProcessor using create_processor function and then defining the type of processing/aggregation you want to use. For defining a processor rollingWindow, groupBy, add_computation, create functions need to be chained.
    2. Fetching the aggregated features from the processor using get_for_item and get functions.
    """
    def get_for_items(self, frontendJsonTensor:Tensor)->Tensor:
        """
        Returns the aggregrated features tensor. The data type of the tensor will be same as defined in create_processor function.
        Shape of the tensor will be [frontJsonTensor.length(), rollingWindow.length()].

        Parameters
        ----------
        frontendJsonTensor : Tensor
            Its a tensor of json objects. The aggregated feature fetched will correspond to each of the json's present in the tensor.

        Returns
        ----------
        aggregratedFeatures : Tensor
            Aggregrated features output tensor.
        """
        pass

    def get(self, group:list|Tensor)->Tensor:
        """
        Returns the aggregrated features of the columns which were defined in groupByColumns function.
        Suppose the eventProcessor was created with groupBy of two columns lets say categoryId and brand, then to fetch the aggregated value of the group, this function will be called with group=["Mobile", "Samsung"].
        Parameters
        ----------
        group : list|Tensor
            List/Tensor of the group that you want to get the aggregrated value for.

        Returns
        ----------
        aggregratedFeatures : Tensor
            Aggregrated features output tensor.
        """
        pass


class IntermediateEventProcessor:
    def add_computation(self, aggColumn:str, aggOp:str, defaultValue:int|float):
        """
        Define the aggregration logic to happen on the grouped events. Multiple aggregration logics can be added in the same EventProcessor.

        Parameters
        ----------
        aggColumn : str
            Defines the column to aggregrate on
        aggOp : str
            Defines the aggregate operation to do. Currently supported options are Avg, Sum, Count, Min and Max.
        defaultValue : int|float
            Defines the default value to return in case of no evetns to aggregrate on.
        """
        return self

    def groupBy(self, groupBycolumns: list[str]|Tensor|tuple[str]):
        """
        Add the list of columns the processor should aggregate on.

        Parameters
        ----------
        groupByColumns : list[str] | Tensor | tuple[str]
            The names of the columns you want the processor to aggregate on. The argument is a list/tensor so that we can support Group By on multiple
        """
        return self

    def rollingWindow(self, rollingWindow:list[int|float]|Tensor|tuple[int]):
        """
        Adds rolling window to the event processor.

        Parameters
        ----------
        rollingWindow : list[int]|Tensor|tuple[int]
            Time window of the last n seconds that you want to aggregrate on.
        """
        return self

    def create()->EventProcessor:
        """
        Create an object of EventProcessor class. Cannot add rolling_window, groupBy columns and computations after this function is called.

        Returns
        ----------
        eventProcessor : EventProcessor
            An object of EventProcessor class.
        """
        pass

class FilteredDataframe:
    def fetch(self, columnName:str,dtype:str)->Tensor:
        """
        Fetch the column values present in filtered events.

        Parameters
        ----------
        columnName : str
            Column for which the values need to be fetched.
        dtype : str
            The data type in which the values are to be fetched.

        Returns
        ----------
        featureTensor : Tensor
            Output tensor of shape n, where n is the number of filtered out events.
        """
        pass

    def num_keys()->int:
        """
        Get the number of events that were filtered.

        Returns
        ----------
        num : int
            Number of events.
        """
        pass

class Dataframe:
    def __init__(self, eventSchema: dict)->'Dataframe':
            """
            An object of Dataframe class can be used to interact with the on-device event store in which the events are being populated by frontend interactions of the user. Dataframe object can be used for the following
            1. fetch events using filter_all or filter_by_function functions.
            2. create a pre-processor using processor function.

            Parameters
            ----------
            eventSchema : str
                Event schema. For e.g.
                {
                    "column1": "int32",
                    "column2": "float",
                    "column3": "string"
                }
            """
            pass

    def filter_all()->FilteredDataframe:
        """
        Stores all the events present on the device for a specific eventStoreName (defined in event_store function) in the RawEventStore.

        Returns
        ----------
        filterEventStore: FilteredEventStore
            Event Store will all the events stored.
        """
        pass

    def filter_by_function(self, filterFunction:FunctionType)-> FilteredDataframe:
        """
        Filter the events stored on device by executing the function on all the events. Events which return on function execution will be stored in the RawEventStore.

        Parameters
        ----------
        filterFunction : function
            Name of the user defined function by which event store needs to be filtered. the function should take as an argument a single event of the event store and return true or false based on whether it needs to be filtered in or out.

        Returns
        ----------
        filterEventStore: FilteredEventStore
            Event Store will only the events which were filtered by the filterFunction.
        """
        pass

    def processor(self, dataType:str)->IntermediateEventProcessor:
        """
        Create an intermediate EventProcessor object, which is then be used to add processing and aggregration logic over the dataframe.

        Parameters
        ----------
        dataType : str
            Data type in which the outputs of the processed events are fetched.
        """

        pass

    def append(self, event: dict)-> None:
        """
        Add an event to the dataframe.

        Parameters
        ----------
        event : dict
            Event to be added to the dataframe.
        """

        pass
