#!/usr/bin/env ruby

require 'rubygems'
require 'osc'

class K8056
  def initialize(host='localhost', port='8056', card=1)
    @client = OSC::SimpleClient.new(host, port)
    @card = card.to_i
  end

  def []=(relay,value)
    if value
      set_relay(relay)
    else
      clear_relay(relay)
    end
  end

  def set(value)
    @client.send(
      OSC::Message.new('/k8056/set', 'ii', @card, value.to_i)
    )
  end

  def set_relay(relay)
    @client.send(
      OSC::Message.new('/k8056/set_relay', 'ii', @card, relay.to_i)
    )
  end

  def clear_relay(relay)
    @client.send(
      OSC::Message.new('/k8056/clear_relay', 'ii', @card, relay.to_i)
    )
  end

  def toggle_relay(relay)
    @client.send(
      OSC::Message.new('/k8056/toggle_relay', 'ii', @card, relay.to_i)
    )
  end

  def emergency_stop
    @client.send(
      OSC::Message.new('/k8056/emergency_stop')
    )
  end

  def display_address
    @client.send(
      OSC::Message.new('/k8056/display_address')
    )
  end

  def set_address(new_address)
    @client.send(
      OSC::Message.new('/k8056/set_address', @card, new_address.to_i)
    )
  end

  def display_address
    @client.send(
      OSC::Message.new('/k8056/reset_address')
    )
  end

end


if __FILE__ == $0
  k = K8056.new
  (0..0xff).each {|i| k.set(i); sleep 0.3}
end
