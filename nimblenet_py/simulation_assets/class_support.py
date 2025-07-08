# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

class WorkflowNode:
    globalId = 1
    def __init__(self,func):
        self.func = func
        self.id = str(WorkflowNode.globalId)
        WorkflowNode.globalId = WorkflowNode.globalId + 1
    def execute_node(self, state):
        self.func(state)

class Node:
    def __init__(self,id):
        self.id = id

class Workflow:
    
    def startNode():
        return Node("start")

    def endNode():
        return Node("end")
    
    def __init__(self):
        self.id2NodeMap = {}
        self.nextNode={}
    def add_node(self,func):
        newNode = WorkflowNode(func)
        self.id2NodeMap[newNode.id] = newNode
        return newNode

    def add_edges(self,edges):
        #writing
        for edge in edges:
            a,b = edge
            self.nextNode[a.id]=b

    def execute(self,stateObject):
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

class State:
    def __init__(self, n):
        self.val = n

    def print(self):
        print("state val is ", self.val)

def add_3(state):    
    print("add_3 called")
    state.val = state.val +3
    state.print()

def square(state):    
    print("square called")
    state.val = state.val**2
    state.print()

def minus_2(state):    
    print("minus_2 called")
    state.val = state.val -2
    state.print()

w = Workflow()
addNode = w.add_node(add_3)
squareNode = w.add_node(square)
minusNode = w.add_node(minus_2)

w.add_edges(
            [
                (Workflow.startNode(), addNode),
                (addNode, squareNode),
                (squareNode, minusNode),
                (minusNode, Workflow.endNode())
            ]
        )

def simple_func(val):
    return (val+3)**2 -2

def test_workflow(inp):
    state = State(inp["input"])
    w.execute(state)
    actualResult = simple_func(inp["input"])
    callback = inp["assertion_callback"]
    callback({"workflow_output": state.val, "actual_output":actualResult})
    return {}
