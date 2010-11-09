#!/usr/bin/env ruby

require 'rubygems'
require 'osc'
include OSC

class K8056
  def initialize(host='localhost', port='8056', card=1)
    @client = SimpleClient.new(host, port)
    @card = card.to_i
  end
  
  
  def set_relay(relay)
    @client.send(
      Message.new('/k8056/set_relay', 'ii', @card, relay.to_i)
    )
  end

  def clear_relay(relay)
    @client.send(
      Message.new('/k8056/clear_relay', 'ii', @card, relay.to_i)
    )
  end

  def toggle_relay(relay)
    @client.send(
      Message.new('/k8056/toggle_relay', 'ii', @card, relay.to_i)
    )
  end
end


if __FILE__ == $0
  k = K8056.new
  k.toggle_relay(1)
end
