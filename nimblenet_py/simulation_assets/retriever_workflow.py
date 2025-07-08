# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

class WorkflowNode:
    globalId = 1

    def __init__(self, func):
        self.func = func
        self.id = str(WorkflowNode.globalId)
        WorkflowNode.globalId = WorkflowNode.globalId + 1

    def execute_node(self, state):
        self.func(state)


class Node:
    def __init__(self, id):
        self.id = id


class Workflow:
    def startNode():
        return Node("start")

    def endNode():
        return Node("end")

    def __init__(self):
        self.id2NodeMap = {}
        self.nextNode = {}

    def add_node(self, func):
        newNode = WorkflowNode(func)
        self.id2NodeMap[newNode.id] = newNode
        return newNode

    def add_edges(self, edges):
        # writing
        for edge in edges:
            a, b = edge
            self.nextNode[a.id] = b

    def execute(self, stateObject):
        # get next node from start Node
        currentNode = self.nextNode["start"]

        while True:
            currentNode = self.id2NodeMap[currentNode.id]
            currentNode.execute_node(stateObject)
            nextNode = self.nextNode[currentNode.id]
            if nextNode.id == "end":
                break
            currentNode = nextNode
        print("workflow complete")


def min(a, b):
    if a < b:
        return a
    return b


# Test streaming JSON APIs

from delitepy import nimblenet as nm
from delitepy import nimblenetInternalTesting as nmi


class Retriever:
    def __init__(self, m1, m2, items):
        self.embeddingModel = m1
        self.embeddingStoreModel = m2
        self.items = items

    def topk(self, query, k):
        queryTensor = nm.tensor([query], "string")
        (embedding,) = self.embeddingModel.run(queryTensor)
        scores, indices = self.embeddingStoreModel.run(embedding)
        rearrangedItems = []
        correspondingScores = []
        for i in range(min(k, len(self.items))):
            index = indices[i]
            rearrangedItems.append(self.items[index])
            correspondingScores.append(scores[i])
        return correspondingScores, rearrangedItems


PROMPT_START = """<|start_header_id|>system<|end_header_id|>You are a helpful assistant to the user who is trying to buy grocery items. List the ingredients needed to cook user's dish given in the question.
Give output exactly following the below structure, :
{
"description" : summary in 10 words,
"items": [...]
}
<|eot_id|>
<|start_header_id|>user<|end_header_id|>question: """

PROMPT_END = """<|eot_id|>
<|start_header_id|>assistant<|end_header_id|>
"""

retriever = None
llm = nm.llm("llama-3")
# llm = nm.llm("phi-3")


def offline_initialize(inp):
    m1 = nm.Model(inp["embeddingModel"])
    m2 = nm.Model(inp["embeddingStoreModel"])
    items = nm.parse_json(inp["items"])
    retriever = Retriever(m1, m2, items)
    return {}


def run_llm(state):
    print("Running LLM")
    charStream = llm.prompt(PROMPT_START + state.userPrompt + PROMPT_END)
    # state.llmStream = charStream
    state.llmStream = charStream.skip_text_and_get_json_stream()


def get_description(state):
    # while not state.llmStream.finished():
    #     descriptionCallback = state.descriptionCallback
    #     descriptionCallback({"description": state.llmStream.next()})
    description = state.llmStream.get_blocking_str("description")
    descriptionCallback = state.descriptionCallback
    descriptionCallback({"description": description})


def get_items(state):
    itemsIterator = state.llmStream.get_blocking("items").iterator()
    while True:
        next_stream = itemsIterator.next_blocking()
        if next_stream:
            next_stream.wait_for_completion()
            item_name = str(next_stream)
            scores, docs = retriever.topk(item_name, 1)
            if scores[0] > 0.5:
                itemCallback = state.itemCallback
                itemCallback(
                    {
                        "llm_output": item_name,
                        "item": docs[0],
                        "relevance_score": scores[0],
                    }
                )
        else:
            # Item Stream has ended
            break


class State:
    def __init__(self, userPrompt, descriptionCallback, itemCallback):
        self.userPrompt = userPrompt
        self.descriptionCallback = descriptionCallback
        self.itemCallback = itemCallback
        self.llmStream = None


w = Workflow()
llmNode = w.add_node(run_llm)
getDescriptionNode = w.add_node(get_description)
getItemsNode = w.add_node(get_items)

w.add_edges(
    [
        (Workflow.startNode(), llmNode),
        (llmNode, getDescriptionNode),
        # (getDescriptionNode, Workflow.endNode()),
        (getDescriptionNode, getItemsNode),
        (getItemsNode, Workflow.endNode()),
    ]
)


def run_workflow(inp):
    print("Running workflow")
    state = State(inp["userPrompt"], inp["descriptionCallback"], inp["itemCallback"])
    w.execute(state)

    # outputs are returned to frontend using the callback functions they provide, so no output is needed here
    return {}
