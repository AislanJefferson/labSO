class FifoPolicy:
  def __init__(self):
    self.fifo = []

  def put(self, frameId):
    """Allocates this frameId for some page"""
    self.fifo.append(frameId)

  def evict(self):
    """Deallocates a frame from the physical memory and returns its frameId"""
    return self.fifo.pop(0)