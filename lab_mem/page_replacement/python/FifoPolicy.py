class FifoPolicy:
  def __init__(self):
    self.fifo = []

  def put(self, frameId):
    self.fifo.append(frameId)

  def evict(self):
    return self.fifo.pop(0)

  def clock(self):
    pass

  def access(self, frameId, isWrite):
    pass