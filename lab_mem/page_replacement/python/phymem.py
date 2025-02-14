# This is the only file you must implement

# This file will be imported from the main code. The PhysicalMemory class
# will be instantiated with the algorithm received from the input. You may edit
# this file as you which

# NOTE: there may be methods you don't need to modify, you must decide what
# you need...
from FifoPolicy import FifoPolicy
from SecChancePolicy import SecChancePolicy
from NRUPolicy import NRUPolicy
from LRUPolicy import LRUPolicy
from AgingPolicy import AgingPolicy
from RandomPolicy import RandomPolicy

class PhysicalMemory:
  ALGORITHM_AGING_NBITS = 8
  """How many bits to use for the Aging algorithm"""
  def __init__(self, algorithm):
    assert algorithm in {"fifo", "nru", "aging", "second-chance", "random", "lru"}
    self.algorithm = algorithm
    if(algorithm == "fifo"):
      self.policy = FifoPolicy()
    elif(algorithm == "second-chance"):
      self.policy = SecChancePolicy()
    elif(algorithm == "nru"):
      self.policy = NRUPolicy()
    elif(algorithm == "aging"):
      self.policy = AgingPolicy()
    elif(algorithm == "lru"):
      self.policy = LRUPolicy()
    else:
      self.policy = RandomPolicy()


  def put(self, frameId):
    """Allocates this frameId for some page"""
    # Notice that in the physical memory we don't care about the pageId, we only
    # care about the fact we were requested to allocate a certain frameId
    self.policy.put(frameId)

  def evict(self):
    """Deallocates a frame from the physical memory and returns its frameId"""
    # You may assume the physical memory is FULL so we need space!
    # Your code must decide which frame to return, according to the algorithm
    return self.policy.evict();

  def clock(self):
    """The amount of time we set for the clock has passed, so this is called"""
    # Clear the reference bits (and/or whatever else you think you must do...)
    return self.policy.clock()

  def access(self, frameId, isWrite):
    """A frameId was accessed for read/write (if write, isWrite=True)"""
    return self.policy.access(frameId,isWrite)
