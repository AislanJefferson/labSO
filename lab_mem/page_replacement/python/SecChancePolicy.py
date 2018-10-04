from FifoPolicy import FifoPolicy
class SecChancePolicy(FifoPolicy):
  def put(self, frameId):
    """Allocates this frameId for some page"""
    self.fifo.append([0, frameId])

  def evict(self):
    """Deallocates a frame from the physical memory and returns its frameId"""
    item = self.fifo.pop(0)
    if(item[0] == 0): 
    	item[0] += 1
    	self.fifo.append(item)
    elif(item[0] == 1):
    	return item[1]
    else:
    	return evict()