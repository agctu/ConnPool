# connection_pool

This is a simple implementation of a connection pool.

I want it to be simple and elegant.


## LifeTime Analysis

- Idle: After construction, the pool is not running and is not available for use. Calling `ConnectionPool::stop` also put the pool at this stage, when in *Busy* stage.
  In this stage, this pool is configurable.

- Busy: When in idle stage, calling `ConnectionPool::start` changes it's stage to *Busy*.
  In this stage, the pool is populated with connections. And its `ConnectionPool::getConn` and `ConnectionPool::relConn` methods can be called to get or release connection.

## Thread Safety Analysis

All methods should be thread-safe.
