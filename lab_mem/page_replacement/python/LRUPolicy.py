from FifoPolicy import FifoPolicy
class LRUPolicy(FifoPolicy):

  def access(self, frameId, isWrite):
    for frame in self.fifo:
        if(frameId == frame):
        	self.fifo.remove(frame)
        	self.fifo.append(frame)
                
