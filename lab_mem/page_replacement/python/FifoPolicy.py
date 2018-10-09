class FifoPolicy:
  def __init__(self):
    self.fifo = []

  def put(self, frameId):
    """Allocates this frameId for some page"""
    self.fifo.append(frameId)

  def evict(self):
    """Deallocates a frame from the physical memory and returns its frameId"""
    return self.fifo.pop(0)

  def clock(self):
    """The amount of time we set for the clock has passed, so this is called"""
    # Clear the reference bits (and/or whatever else you think you must do...)
    pass

  def access(self, frameId, isWrite):
    """A frameId was accessed for read/write (if write, isWrite=True)"""
    pass